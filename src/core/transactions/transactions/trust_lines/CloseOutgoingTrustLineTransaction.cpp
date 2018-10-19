#include "CloseOutgoingTrustLineTransaction.h"

CloseOutgoingTrustLineTransaction::CloseOutgoingTrustLineTransaction(
    const NodeUUID &nodeUUID,
    CloseOutgoingTrustLineMessage::Shared message,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    TopologyCacheManager *topologyCacheManager,
    MaxFlowCacheManager *maxFlowCacheManager,
    Keystore* keystore,
    TrustLinesInfluenceController *trustLinesInfluenceController,
    Logger &logger)
    noexcept:

    BaseTrustLineTransaction(
        BaseTransaction::CloseOutgoingTrustLineTransactionType,
        message->transactionUUID(),
        nodeUUID,
        message->equivalent(),
        message->senderUUID,
        manager,
        storageHandler,
        keystore,
        trustLinesInfluenceController,
        logger),
    mMessage(message),
    mTopologyCacheManager(topologyCacheManager),
    mMaxFlowCacheManager(maxFlowCacheManager)
{
    mAuditNumber = mTrustLines->auditNumber(mContractorUUID) + 1;
}

TransactionResult::SharedConst CloseOutgoingTrustLineTransaction::run()
{
    info() << "sender: " << mContractorUUID;

    if (mContractorUUID == mNodeUUID) {
        warning() << "Attempt to launch transaction against itself was prevented.";
        return resultDone();
    }

    if (!mTrustLines->trustLineIsPresent(mContractorUUID)) {
        warning() << "Trust line is absent.";
        return sendAuditErrorConfirmation(
            ConfirmationMessage::ErrorShouldBeRemovedFromQueue);
    }

    if (mTrustLines->trustLineState(mContractorUUID) != TrustLine::Active) {
        warning() << "Invalid TL state " << mTrustLines->trustLineState(mContractorUUID);
        return sendAuditErrorConfirmation(
            ConfirmationMessage::ErrorShouldBeRemovedFromQueue);
    }

    // todo maybe check in storage (keyChain)
    if (!mTrustLines->trustLineOwnKeysPresent(mContractorUUID)) {
        warning() << "There are no own keys";
        return sendAuditErrorConfirmation(
            ConfirmationMessage::OwnKeysAbsent);
    }

    // todo maybe check in storage (keyChain)
    if (!mTrustLines->trustLineContractorKeysPresent(mContractorUUID)) {
        warning() << "There are no contractor keys";
        return sendAuditErrorConfirmation(
            ConfirmationMessage::ContractorKeysAbsent);
    }

    mPreviousOutgoingAmount = mTrustLines->outgoingTrustAmount(mContractorUUID);
    mPreviousState = mTrustLines->trustLineState(mContractorUUID);

    // Trust line must be created (or updated) in the internal storage.
    // Also, history record must be written about this operation.
    // Both writes must be done atomically, so the IO transaction is used.
    auto ioTransaction = mStorageHandler->beginTransaction();
    try {
        // note: io transaction would commit automatically on destructor call.
        // there is no need to call commit manually.
        mTrustLines->setTrustLineState(
            mContractorUUID,
            TrustLine::AuditPending);

        mTrustLines->closeOutgoing(
            mContractorUUID);

        auto keyChain = mKeysStore->keychain(
            mTrustLines->trustLineID(mContractorUUID));
        auto contractorSerializedAuditData = getContractorSerializedAuditData();
        if (!keyChain.checkSign(
                ioTransaction,
                contractorSerializedAuditData.first,
                contractorSerializedAuditData.second,
                mMessage->signature(),
                mMessage->keyNumber())) {
            warning() << "Contractor didn't sign message correct";
            return sendAuditErrorConfirmation(
                ConfirmationMessage::ErrorShouldBeRemovedFromQueue);
        }
        info() << "Signature is correct";

        auto serializedAuditData = getOwnSerializedAuditData();
        mOwnSignatureAndKeyNumber = keyChain.sign(
            ioTransaction,
            serializedAuditData.first,
            serializedAuditData.second);

        keyChain.saveFullAudit(
            ioTransaction,
            mAuditNumber,
            mOwnSignatureAndKeyNumber.second,
            mOwnSignatureAndKeyNumber.first,
            mMessage->keyNumber(),
            mMessage->signature(),
            mTrustLines->incomingTrustAmount(
                mContractorUUID),
            mTrustLines->outgoingTrustAmount(
                mContractorUUID),
            mTrustLines->balance(mContractorUUID));

        mTrustLines->setTrustLineAuditNumberAndMakeActive(
            mContractorUUID,
            mAuditNumber);
        mTrustLines->resetTrustLineTotalReceiptsAmounts(
            mContractorUUID);

        populateHistory(ioTransaction, TrustLineRecord::RejectingOutgoing);
        mTopologyCacheManager->resetInitiatorCache();
        mMaxFlowCacheManager->clearCashes();
        info() << "Outgoing trust line to the node " << mContractorUUID
               << " has been successfully closed by remote node.";


#ifdef TESTS
        mTrustLinesInfluenceController->testThrowExceptionOnTLModifyingStage();
        mTrustLinesInfluenceController->testTerminateProcessOnTLModifyingStage();
#endif

    } catch (IOError &e) {
        ioTransaction->rollback();
        // return closed TL
        mTrustLines->setOutgoing(
            mContractorUUID,
            mPreviousOutgoingAmount);
        mTrustLines->setTrustLineState(
            mContractorUUID,
            mPreviousState);
        warning() << "Attempt to close outgoing trust line to the node " << mContractorUUID << " failed. "
               << "IO transaction can't be completed. "
               << "Details are: " << e.what();
        // Rethrowing the exception,
        // because the TA can't finish properly and no result may be returned.
        throw e;
    }

    // Sending confirmation back.
    sendMessage<AuditResponseMessage>(
        mContractorUUID,
        mEquivalent,
        mNodeUUID,
        mTransactionUUID,
        mOwnSignatureAndKeyNumber.second,
        mOwnSignatureAndKeyNumber.first);
    info() << "Send audit message signed by key " << mOwnSignatureAndKeyNumber.second;

    trustLineActionSignal(
        mContractorUUID,
        mEquivalent,
        false);

    return resultDone();
}

const string CloseOutgoingTrustLineTransaction::logHeader() const
noexcept
{
    stringstream s;
    s << "[CloseOutgoingTrustLineTA: " << currentTransactionUUID() << " " << mEquivalent << "]";
    return s.str();
}

void CloseOutgoingTrustLineTransaction::populateHistory(
    IOTransaction::Shared ioTransaction,
    TrustLineRecord::TrustLineOperationType operationType)
{
#ifndef TESTS
    auto record = make_shared<TrustLineRecord>(
        mTransactionUUID,
        operationType,
        mContractorUUID);

    ioTransaction->historyStorage()->saveTrustLineRecord(
        record,
        mEquivalent);
#endif
}
