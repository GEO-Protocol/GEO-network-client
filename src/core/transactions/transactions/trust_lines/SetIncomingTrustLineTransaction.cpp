#include "SetIncomingTrustLineTransaction.h"

SetIncomingTrustLineTransaction::SetIncomingTrustLineTransaction(
    const NodeUUID &nodeUUID,
    SetIncomingTrustLineMessage::Shared message,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    TopologyTrustLinesManager *topologyTrustLinesManager,
    TopologyCacheManager *topologyCacheManager,
    MaxFlowCacheManager *maxFlowCacheManager,
    SubsystemsController *subsystemsController,
    Keystore *keystore,
    VisualInterface *visualInterface,
    TrustLinesInfluenceController *trustLinesInfluenceController,
    Logger &logger)
    noexcept:

    BaseTrustLineTransaction(
        BaseTransaction::SetIncomingTrustLineTransaction,
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
    mAuditNumber = mTrustLines->auditNumber(mContractorUUID) + 1;
}

TransactionResult::SharedConst SetIncomingTrustLineTransaction::run()
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

    mPreviousIncomingAmount = mTrustLines->incomingTrustAmount(mContractorUUID);
    mPreviousState = mTrustLines->trustLineState(mContractorUUID);

    // Trust line must be created (or updated) in the internal storage.
    // Also, history record must be written about this operation.
    // Both writes must be done atomically, so the IO transaction is used.
    auto ioTransaction = mStorageHandler->beginTransaction();

    // if contractor in black list we should reject operation with TL
    if (ioTransaction->blackListHandler()->checkIfNodeExists(mContractorUUID)) {
        warning() << "Contractor " << mContractorUUID << " is in black list. Transaction rejected";
        return sendAuditErrorConfirmation(
            ConfirmationMessage::ContractorBanned);
    }

    try {
        // note: io transaction would commit automatically on destructor call.
        // there is no need to call commit manually.
        auto kOperationResult = mTrustLines->setIncoming(
            mContractorUUID,
            mMessage->amount());

        mTrustLines->setTrustLineState(
            mContractorUUID,
            TrustLine::AuditPending);

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

        switch (kOperationResult) {
        case TrustLinesManager::TrustLineOperationResult::Opened: {
            populateHistory(ioTransaction, TrustLineRecord::Accepting);
            mTopologyCacheManager->resetInitiatorCache();
            mMaxFlowCacheManager->clearCashes();
            info() << "Incoming trust line from the node " << mContractorUUID
                   << " has been successfully initialised with " << mMessage->amount();
            break;
        }

        case TrustLinesManager::TrustLineOperationResult::Updated: {
            populateHistory(ioTransaction, TrustLineRecord::Updating);
            mTopologyCacheManager->resetInitiatorCache();
            mMaxFlowCacheManager->clearCashes();
            info() << "Incoming trust line from the node " << mContractorUUID
                   << " has been successfully set to " << mMessage->amount();
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

#ifdef TESTS
        mTrustLinesInfluenceController->testThrowExceptionOnTLModifyingStage();
        mTrustLinesInfluenceController->testTerminateProcessOnTLModifyingStage();
#endif

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
        mTrustLines->setTrustLineState(
            mContractorUUID,
            mPreviousState);
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

const string SetIncomingTrustLineTransaction::logHeader() const
    noexcept
{
    stringstream s;
    s << "[SetIncomingTrustLineTA: " << currentTransactionUUID() << " " << mEquivalent << "]";
    return s.str();
}

void SetIncomingTrustLineTransaction::populateHistory(
    IOTransaction::Shared ioTransaction,
    TrustLineRecord::TrustLineOperationType operationType)
{
#ifndef TESTS
    auto record = make_shared<TrustLineRecord>(
        mTransactionUUID,
        operationType,
        mContractorUUID,
        mMessage->amount());

    ioTransaction->historyStorage()->saveTrustLineRecord(
        record,
        mEquivalent);
#endif
}
