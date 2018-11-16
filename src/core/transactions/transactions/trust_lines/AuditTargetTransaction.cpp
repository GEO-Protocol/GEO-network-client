#include "AuditTargetTransaction.h"

AuditTargetTransaction::AuditTargetTransaction(
    const NodeUUID &nodeUUID,
    AuditMessage::Shared message,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    Keystore *keystore,
    TopologyTrustLinesManager *topologyTrustLinesManager,
    TopologyCacheManager *topologyCacheManager,
    MaxFlowCacheManager *maxFlowCacheManager,
    SubsystemsController *subsystemsController,
    TrustLinesInfluenceController *trustLinesInfluenceController,
    VisualInterface *visualInterface,
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
    mMessage(message),
    mTopologyTrustLinesManager(topologyTrustLinesManager),
    mTopologyCacheManager(topologyCacheManager),
    mMaxFlowCacheManager(maxFlowCacheManager),
    mSubsystemsController(subsystemsController),
    mVisualInterface(visualInterface)
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
        mTrustLines->trustLineID(mContractorUUID));

    if (mAuditNumber - mMessage->auditNumber() == 1) {
        info() << "Contractor send current audit " << mMessage->auditNumber();
        mOwnSignatureAndKeyNumber = keyChain.getCurrentAuditSignatureAndKeyNumber(ioTransaction);
        // Sending confirmation back.
        sendMessageWithTemporaryCaching<AuditResponseMessage>(
            mContractorUUID,
            Message::TrustLines_Audit,
            kWaitMillisecondsForResponse / 1000 * kMaxCountSendingAttempts,
            mEquivalent,
            mNodeUUID,
            currentTransactionUUID(),
            mOwnSignatureAndKeyNumber.second,
            mOwnSignatureAndKeyNumber.first);
        info() << "Send audit again message signed by key " << mOwnSignatureAndKeyNumber.second;
        return resultDone();
    }

    mPreviousIncomingAmount = mTrustLines->incomingTrustAmount(mContractorUUID);
    mPreviousOutgoingAmount = mTrustLines->outgoingTrustAmount(mContractorUUID);
    mPreviousState = mTrustLines->trustLineState(mContractorUUID);

    try {
        // note: io transaction would commit automatically on destructor call.
        // there is no need to call commit manually.

        if (mMessage->outgoingAmount() != mTrustLines->incomingTrustAmount(mContractorUUID)) {
            info() << "setIncomingTrustLineAmount " << mMessage->outgoingAmount();
            // if contractor in black list we should reject operation with TL
            if (ioTransaction->blackListHandler()->checkIfNodeExists(mContractorUUID)) {
                warning() << "Contractor " << mContractorUUID << " is in black list. Transaction rejected";
                return sendAuditErrorConfirmation(
                    ConfirmationMessage::ContractorBanned);
            }
            setIncomingTrustLineAmount(ioTransaction);
        }

        if (mMessage->incomingAmount() == TrustLine::kZeroAmount()
            and mTrustLines->outgoingTrustAmount(mContractorUUID) != TrustLine::kZeroAmount()) {
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
                mContractorUUID),
            mTrustLines->outgoingTrustAmount(
                mContractorUUID),
            mTrustLines->balance(mContractorUUID));

        mTrustLines->setTrustLineAuditNumber(
            mContractorUUID,
            mAuditNumber);
        mTrustLines->resetTrustLineTotalReceiptsAmounts(
            mContractorUUID);

#ifdef TESTS
        mTrustLinesInfluenceController->testThrowExceptionOnTargetStage(
            BaseTransaction::AuditTargetTransactionType);
        mTrustLinesInfluenceController->testTerminateProcessOnTargetStage(
            BaseTransaction::AuditTargetTransactionType);
#endif

    } catch (ValueError &) {
        warning() << "Attempt to set incoming trust line from the node " << mContractorUUID << " failed. "
                  << "Cannot open TL with zero amount.";
        return sendAuditErrorConfirmation(
            ConfirmationMessage::ErrorShouldBeRemovedFromQueue);
    } catch (IOError &e) {
        ioTransaction->rollback();
        mTrustLines->setIncoming(
            mContractorUUID,
            mPreviousIncomingAmount);
        mTrustLines->setOutgoing(
            mContractorUUID,
            mPreviousOutgoingAmount);
        mTrustLines->setTrustLineState(
            mContractorUUID,
            mPreviousState);
        warning() << "Attempt to change trust line to the node " << mContractorUUID << " failed. "
                  << "IO transaction can't be completed. "
                  << "Details are: " << e.what();

        // Rethrowing the exception,
        // because the TA can't finish properly and no result may be returned.
        throw e;
    }

    // Sending confirmation back.
    sendMessageWithTemporaryCaching<AuditResponseMessage>(
        mContractorUUID,
        Message::TrustLines_Audit,
        kWaitMillisecondsForResponse / 1000 * kMaxCountSendingAttempts,
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

void AuditTargetTransaction::setIncomingTrustLineAmount(
    IOTransaction::Shared ioTransaction)
{
    auto kOperationResult = mTrustLines->setIncoming(
        mContractorUUID,
        mMessage->outgoingAmount());
    switch (kOperationResult) {
        case TrustLinesManager::TrustLineOperationResult::Opened: {
            populateHistory(ioTransaction, TrustLineRecord::Accepting);
            mTopologyCacheManager->resetInitiatorCache();
            mMaxFlowCacheManager->clearCashes();
            info() << "Incoming trust line from the node " << mContractorUUID
                   << " has been successfully initialised with " << mMessage->outgoingAmount();
            break;
        }

        case TrustLinesManager::TrustLineOperationResult::Updated: {
            populateHistory(ioTransaction, TrustLineRecord::Updating);
            mTopologyCacheManager->resetInitiatorCache();
            mMaxFlowCacheManager->clearCashes();
            info() << "Incoming trust line from the node " << mContractorUUID
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
            info() << "Incoming trust line from the node " << mContractorUUID
                   << " has been successfully closed.";
            break;
        }

        case TrustLinesManager::TrustLineOperationResult::NoChanges: {
            // It is possible, that set trust line request will arrive twice or more times,
            // but only first processed update must be written to the trust lines history.
            info() << "Incoming trust line from the node " << mContractorUUID
                   << " has not been changed.";
            break;
        }
    }

    if (mSubsystemsController->isWriteVisualResults()) {
        if (kOperationResult == TrustLinesManager::TrustLineOperationResult::Opened) {
            stringstream s;
            s << VisualResult::IncomingTrustLineOpen << kTokensSeparator
              << microsecondsSinceUnixEpoch() << kTokensSeparator
              << currentTransactionUUID() << kTokensSeparator
              << mContractorUUID << kCommandsSeparator;
            auto message = s.str();

            try {
                mVisualInterface->writeResult(
                    message.c_str(),
                    message.size());
            } catch (IOError &e) {
                error() << "SetIncomingTrustLineTransaction: "
                        "Error occurred when visual result has accepted. Details: " << e.message();
            }
        }
        if (kOperationResult == TrustLinesManager::TrustLineOperationResult::Closed) {
            stringstream s;
            s << VisualResult::IncomingTrustLineClose << kTokensSeparator
              << microsecondsSinceUnixEpoch() << kTokensSeparator
              << currentTransactionUUID() << kTokensSeparator
              << mContractorUUID << kCommandsSeparator;
            auto message = s.str();

            try {
                mVisualInterface->writeResult(
                    message.c_str(),
                    message.size());
            } catch (IOError &e) {
                error() << "SetIncomingTrustLineTransaction: "
                        "Error occurred when visual result has accepted. Details: " << e.message();
            }
        }
    }
}

void AuditTargetTransaction::closeOutgoingTrustLine(
    IOTransaction::Shared ioTransaction)
{
    mTrustLines->closeOutgoing(
        mContractorUUID);
    populateHistory(ioTransaction, TrustLineRecord::RejectingOutgoing);
    mTopologyCacheManager->resetInitiatorCache();
    mMaxFlowCacheManager->clearCashes();
    info() << "Outgoing trust line to the node " << mContractorUUID
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