#include "AuditTargetTransaction.h"

AuditTargetTransaction::AuditTargetTransaction(
    const NodeUUID &nodeUUID,
    AuditMessage::Shared message,
    ContractorsManager *contractorsManager,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    Keystore *keystore,
    TopologyTrustLinesManager *topologyTrustLinesManager,
    TopologyCacheManager *topologyCacheManager,
    MaxFlowCacheManager *maxFlowCacheManager,
    TrustLinesInfluenceController *trustLinesInfluenceController,
    Logger &logger):
    BaseTrustLineTransaction(
        BaseTransaction::AuditTargetTransactionType,
        message->transactionUUID(),
        nodeUUID,
        message->equivalent(),
        message->senderUUID,
        message->idOnReceiverSide,
        contractorsManager,
        manager,
        storageHandler,
        keystore,
        trustLinesInfluenceController,
        logger),
    mMessage(message),
    mSenderIncomingIP(message->senderIncomingIP()),
    mContractorAddresses(message->senderAddresses),
    mTopologyTrustLinesManager(topologyTrustLinesManager),
    mTopologyCacheManager(topologyCacheManager),
    mMaxFlowCacheManager(maxFlowCacheManager)
{
    mAuditNumber = mTrustLines->auditNumber(message->idOnReceiverSide) + 1;
}

TransactionResult::SharedConst AuditTargetTransaction::run()
{
    info() << "sender: " << mContractorID << "sender incoming IP " << mSenderIncomingIP;

    if (!mContractorsManager->contractorPresent(mContractorID)) {
        warning() << "There is no contractor with requested id";
        return sendAuditErrorConfirmation(
            ConfirmationMessage::ErrorShouldBeRemovedFromQueue);
    }

    if (!mTrustLines->trustLineIsPresent(mContractorID)) {
        warning() << "Trust line is absent.";
        return sendAuditErrorConfirmation(
            ConfirmationMessage::ErrorShouldBeRemovedFromQueue);
    }

    if (mTrustLines->trustLineState(mContractorID) != TrustLine::Active) {
        warning() << "Invalid TL state " << mTrustLines->trustLineState(mContractorID);
        return sendAuditErrorConfirmation(
            ConfirmationMessage::ErrorShouldBeRemovedFromQueue);
    }

    // todo maybe check in storage (keyChain)
    if (!mTrustLines->trustLineOwnKeysPresent(mContractorID)) {
        warning() << "There are no own keys";
        return sendAuditErrorConfirmation(
            ConfirmationMessage::OwnKeysAbsent);
    }

    // todo maybe check in storage (keyChain)
    if (!mTrustLines->trustLineContractorKeysPresent(mContractorID)) {
        warning() << "There are no contractor keys";
        return sendAuditErrorConfirmation(
            ConfirmationMessage::ContractorKeysAbsent);
    }

    if (mAuditNumber < mMessage->auditNumber()) {
        warning() << "Contractor's audit number " << mMessage->auditNumber() << " is greater than own " << mAuditNumber;
        // todo : need correct reaction
    }

    if (mAuditNumber - mMessage->auditNumber() > 1) {
        warning() << "Contractor's audit number is deprecated " << mMessage->auditNumber();
        return sendAuditErrorConfirmation(
           ConfirmationMessage::Audit_IncorrectNumber);
    }

    // Trust line must be created (or updated) in the internal storage.
    // Also, history record must be written about this operation.
    // Both writes must be done atomically, so the IO transaction is used.
    auto ioTransaction = mStorageHandler->beginTransaction();
    auto keyChain = mKeysStore->keychain(
        mTrustLines->trustLineID(mContractorID));

    if (mAuditNumber - mMessage->auditNumber() == 1) {
        info() << "Contractor send current audit " << mMessage->auditNumber();
        mOwnSignatureAndKeyNumber = keyChain.getCurrentAuditSignatureAndKeyNumber(ioTransaction);
        // Sending confirmation back.
        sendMessageWithTemporaryCaching<AuditResponseMessage>(
            mContractorID,
            Message::TrustLines_Audit,
            kWaitMillisecondsForResponse / 1000 * kMaxCountSendingAttempts,
            mEquivalent,
            mNodeUUID,
            mContractorsManager->idOnContractorSide(mContractorID),
            currentTransactionUUID(),
            mOwnSignatureAndKeyNumber.second,
            mOwnSignatureAndKeyNumber.first);
        info() << "Send audit again message signed by key " << mOwnSignatureAndKeyNumber.second;
        return resultDone();
    }

    mPreviousIncomingAmount = mTrustLines->incomingTrustAmount(mContractorID);
    mPreviousOutgoingAmount = mTrustLines->outgoingTrustAmount(mContractorID);
    mPreviousState = mTrustLines->trustLineState(mContractorID);

    try {
        // note: io transaction would commit automatically on destructor call.
        // there is no need to call commit manually.

        if (mMessage->outgoingAmount() != mTrustLines->incomingTrustAmount(mContractorID)) {
            info() << "setIncomingTrustLineAmount " << mMessage->outgoingAmount();
            // todo : check black list if need
            setIncomingTrustLineAmount(ioTransaction);
        }

        if (mMessage->incomingAmount() == TrustLine::kZeroAmount()
            and mTrustLines->outgoingTrustAmount(mContractorID) != TrustLine::kZeroAmount()) {
            info() << "closeOutgoingTrustLine";
            closeOutgoingTrustLine(ioTransaction);
        }

        auto contractorSerializedAuditData = getContractorSerializedAuditData();

        if (!keyChain.checkSign(
                ioTransaction,
                contractorSerializedAuditData.first,
                contractorSerializedAuditData.second,
                mMessage->signature(),
                mMessage->keyNumber())) {
            warning() << "Contractor didn't sign message correct by key number " << mMessage->keyNumber();
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
                mContractorID),
            mTrustLines->outgoingTrustAmount(
                mContractorID),
            mTrustLines->balance(mContractorID));

        mTrustLines->setTrustLineAuditNumber(
            mContractorID,
            mAuditNumber);
        mTrustLines->resetTrustLineTotalReceiptsAmounts(
            mContractorID);

        if (mTrustLines->isTrustLineEmpty(mContractorID) and
                mAuditNumber > TrustLine::kInitialAuditNumber + 1) {
            mTrustLines->setTrustLineState(
                mContractorID,
                TrustLine::Archived,
                ioTransaction);
            keyChain.removeUnusedOwnKeys(ioTransaction);
            mTrustLines->setIsOwnKeysPresent(mContractorID, false);
            keyChain.removeUnusedContractorKeys(ioTransaction);
            mTrustLines->setIsContractorKeysPresent(mContractorID, false);
            info() << "Trust Line become empty";
        } else {
            info() << "All data saved. Now TL is ready for using";
        }

#ifdef TESTS
        mTrustLinesInfluenceController->testThrowExceptionOnTargetStage(
            BaseTransaction::AuditTargetTransactionType);
        mTrustLinesInfluenceController->testTerminateProcessOnTargetStage(
            BaseTransaction::AuditTargetTransactionType);
#endif

    } catch (ValueError &) {
        warning() << "Attempt to set incoming trust line from the node " << mContractorID << " failed. "
                  << "Cannot open TL with zero amount.";
        return sendAuditErrorConfirmation(
            ConfirmationMessage::ErrorShouldBeRemovedFromQueue);
    } catch (IOError &e) {
        ioTransaction->rollback();
        mTrustLines->setIncoming(
            mContractorID,
            mPreviousIncomingAmount);
        mTrustLines->setOutgoing(
            mContractorID,
            mPreviousOutgoingAmount);
        mTrustLines->setTrustLineState(
            mContractorID,
            mPreviousState);
        warning() << "Attempt to change trust line to the node " << mContractorID << " failed. "
                  << "IO transaction can't be completed. "
                  << "Details are: " << e.what();

        // Rethrowing the exception,
        // because the TA can't finish properly and no result may be returned.
        throw e;
    }

    // Sending confirmation back.
    sendMessageWithTemporaryCaching<AuditResponseMessage>(
        mContractorID,
        Message::TrustLines_Audit,
        kWaitMillisecondsForResponse / 1000 * kMaxCountSendingAttempts,
        mEquivalent,
        mNodeUUID,
        mContractorsManager->idOnContractorSide(mContractorID),
        currentTransactionUUID(),
        mOwnSignatureAndKeyNumber.second,
        mOwnSignatureAndKeyNumber.first);
    info() << "Send audit message signed by key " << mOwnSignatureAndKeyNumber.second;

    trustLineActionSignal(
        mContractorID,
        mEquivalent,
        false);

    return resultDone();
}

void AuditTargetTransaction::setIncomingTrustLineAmount(
    IOTransaction::Shared ioTransaction)
{
    auto kOperationResult = mTrustLines->setIncoming(
        mContractorID,
        mMessage->outgoingAmount());
    switch (kOperationResult) {
        case TrustLinesManager::TrustLineOperationResult::Opened: {
            populateHistory(ioTransaction, TrustLineRecord::Accepting);
            mTopologyCacheManager->resetInitiatorCache();
            mMaxFlowCacheManager->clearCashes();
            info() << "Incoming trust line from the node " << mContractorID
                   << " has been successfully initialised with " << mMessage->outgoingAmount();
            break;
        }

        case TrustLinesManager::TrustLineOperationResult::Updated: {
            populateHistory(ioTransaction, TrustLineRecord::Updating);
            mTopologyCacheManager->resetInitiatorCache();
            mMaxFlowCacheManager->clearCashes();
            info() << "Incoming trust line from the node " << mContractorID
                   << " has been successfully set to " << mMessage->outgoingAmount();
            break;
        }

        case TrustLinesManager::TrustLineOperationResult::Closed: {
            populateHistory(ioTransaction, TrustLineRecord::Rejecting);
            // remove this TL from Topology TrustLines Manager
            mTopologyTrustLinesManager->addTrustLine(
                make_shared<TopologyTrustLine>(
                    mNodeUUID,
                    mContractorUUID,
                    make_shared<const TrustLineAmount>(0)));
            mTopologyCacheManager->resetInitiatorCache();
            mMaxFlowCacheManager->clearCashes();
            info() << "Incoming trust line from the node " << mContractorID
                   << " has been successfully closed.";
            break;
        }

        case TrustLinesManager::TrustLineOperationResult::NoChanges: {
            // It is possible, that set trust line request will arrive twice or more times,
            // but only first processed update must be written to the trust lines history.
            info() << "Incoming trust line from the node " << mContractorID
                   << " has not been changed.";
            break;
        }
    }
}

void AuditTargetTransaction::closeOutgoingTrustLine(
    IOTransaction::Shared ioTransaction)
{
    mTrustLines->closeOutgoing(
        mContractorID);
    populateHistory(ioTransaction, TrustLineRecord::RejectingOutgoing);
    mTopologyCacheManager->resetInitiatorCache();
    mMaxFlowCacheManager->clearCashes();
    info() << "Outgoing trust line to the node " << mContractorID
           << " has been successfully closed by remote node.";
}

void AuditTargetTransaction::populateHistory(
    IOTransaction::Shared ioTransaction,
    TrustLineRecord::TrustLineOperationType operationType)
{
#ifndef TESTS
    TrustLineRecord::Shared record;
    if (operationType != TrustLineRecord::RejectingOutgoing) {
        record = make_shared<TrustLineRecord>(
            mTransactionUUID,
            operationType,
            mContractorUUID,
            mMessage->outgoingAmount());
    } else {
        record = make_shared<TrustLineRecord>(
            mTransactionUUID,
            operationType,
            mContractorUUID);
    }

    ioTransaction->historyStorage()->saveTrustLineRecord(
        record,
        mEquivalent);
#endif
}

const string AuditTargetTransaction::logHeader() const
{
    stringstream s;
    s << "[AuditTargetTA: " << currentTransactionUUID() << " " << mEquivalent << "]";
    return s.str();
}