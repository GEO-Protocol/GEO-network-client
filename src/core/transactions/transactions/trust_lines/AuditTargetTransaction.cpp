#include "AuditTargetTransaction.h"

AuditTargetTransaction::AuditTargetTransaction(
    const NodeUUID &nodeUUID,
    AuditMessage::Shared message,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    Keystore *keystore,
    TrustLinesInfluenceController *trustLinesInfluenceController,
    Logger &logger):
    BaseTrustLineTransaction(
        BaseTransaction::AuditTargetTransactionType,
        message->transactionUUID(),
        nodeUUID,
        message->equivalent(),
        message->senderUUID,
        manager,
        storageHandler,
        keystore,
        trustLinesInfluenceController,
        logger),
    mMessage(message)
{
    mAuditNumber = mTrustLines->auditNumber(message->senderUUID) + 1;
}

TransactionResult::SharedConst AuditTargetTransaction::run()
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

    // Trust line must be created (or updated) in the internal storage.
    // Also, history record must be written about this operation.
    // Both writes must be done atomically, so the IO transaction is used.
    auto ioTransaction = mStorageHandler->beginTransaction();
    auto keyChain = mKeysStore->keychain(
        mTrustLines->trustLineID(mContractorUUID));
    auto contractorSerializedAuditData = getContractorSerializedAuditData();

    try {
        // note: io transaction would commit automatically on destructor call.
        // there is no need to call commit manually.
        mTrustLines->setTrustLineState(
            mContractorUUID,
            TrustLine::AuditPending);
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

#ifdef TESTS
        mTrustLinesInfluenceController->testThrowExceptionOnAuditStage();
        mTrustLinesInfluenceController->testTerminateProcessOnAuditStage();
#endif

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

#ifdef TESTS
        mTrustLinesInfluenceController->testThrowExceptionOnTLModifyingStage();
        mTrustLinesInfluenceController->testTerminateProcessOnTLModifyingStage();
#endif

    } catch (IOError &e) {
        ioTransaction->rollback();
        mTrustLines->setTrustLineState(
            mContractorUUID,
            TrustLine::Active);
        warning() << "Attempt to set incoming trust line to the node " << mContractorUUID << " failed. "
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
        currentTransactionUUID(),
        mOwnSignatureAndKeyNumber.second,
        mOwnSignatureAndKeyNumber.first);
    info() << "Send audit message signed by key " << mOwnSignatureAndKeyNumber.second;

    trustLineActionSignal(
        mContractorUUID,
        mEquivalent,
        false);

    return resultDone();
}

const string AuditTargetTransaction::logHeader() const
{
    stringstream s;
    s << "[AuditTargetTA: " << currentTransactionUUID() << " " << mEquivalent << "]";
    return s.str();
}