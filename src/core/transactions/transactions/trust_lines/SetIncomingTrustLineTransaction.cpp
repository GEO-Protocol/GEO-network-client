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
    VisualInterface *visualInterface,
    bool iAmGateway,
    Logger &logger)
    noexcept:

    BaseTransaction(
        BaseTransaction::SetIncomingTrustLineTransaction,
        message->transactionUUID(),
        nodeUUID,
        message->equivalent(),
        logger),
    mMessage(message),
    mTrustLines(manager),
    mStorageHandler(storageHandler),
    mTopologyTrustLinesManager(topologyTrustLinesManager),
    mTopologyCacheManager(topologyCacheManager),
    mMaxFlowCacheManager(maxFlowCacheManager),
    mSubsystemsController(subsystemsController),
    mVisualInterface(visualInterface),
    mIAmGateway(iAmGateway),
    mSenderIsGateway(false)
{}

SetIncomingTrustLineTransaction::SetIncomingTrustLineTransaction(
    const NodeUUID &nodeUUID,
    SetIncomingTrustLineFromGatewayMessage::Shared message,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    TopologyTrustLinesManager *topologyTrustLinesManager,
    TopologyCacheManager *topologyCacheManager,
    MaxFlowCacheManager *maxFlowCacheManager,
    SubsystemsController *subsystemsController,
    VisualInterface *visualInterface,
    bool iAmGateway,
    Logger &logger)
    noexcept:

    BaseTransaction(
        BaseTransaction::SetIncomingTrustLineTransaction,
        message->transactionUUID(),
        nodeUUID,
        message->equivalent(),
        logger),
    mMessage(message),
    mTrustLines(manager),
    mStorageHandler(storageHandler),
    mTopologyTrustLinesManager(topologyTrustLinesManager),
    mTopologyCacheManager(topologyCacheManager),
    mMaxFlowCacheManager(maxFlowCacheManager),
    mSubsystemsController(subsystemsController),
    mVisualInterface(visualInterface),
    mIAmGateway(iAmGateway),
    mSenderIsGateway(true)
{}

TransactionResult::SharedConst SetIncomingTrustLineTransaction::run()
{
    info() << "sender: " << mMessage->senderUUID;
    if (mMessage->senderUUID == mNodeUUID) {
        warning() << "Attempt to launch transaction against itself was prevented.";
        return resultDone();
    }

    if (!mTrustLines->trustLineIsPresent(mMessage->senderUUID)) {
        warning() << "Trust line is absent.";
        sendMessage<TrustLineConfirmationMessage>(
            mMessage->senderUUID,
            mEquivalent,
            mNodeUUID,
            mMessage->transactionUUID(),
            false,
            ConfirmationMessage::ErrorShouldBeRemovedFromQueue);
        return resultDone();
    }

    if (mTrustLines->trustLineState(mMessage->senderUUID) != TrustLine::Active) {
        warning() << "Invalid TL state " << mTrustLines->trustLineState(mMessage->senderUUID);
        sendMessage<TrustLineConfirmationMessage>(
            mMessage->senderUUID,
            mEquivalent,
            mNodeUUID,
            mMessage->transactionUUID(),
            false,
            ConfirmationMessage::ErrorShouldBeRemovedFromQueue);
        return resultDone();
    }

    // Trust line must be created (or updated) in the internal storage.
    // Also, history record must be written about this operation.
    // Both writes must be done atomically, so the IO transaction is used.
    auto ioTransaction = mStorageHandler->beginTransaction();

    // if contractor in black list we should reject operation with TL
    if (ioTransaction->blackListHandler()->checkIfNodeExists(mMessage->senderUUID)) {
        warning() << "Contractor " << mMessage->senderUUID << " is in black list. Transaction rejected";
        sendMessage<TrustLineConfirmationMessage>(
            mMessage->senderUUID,
            mEquivalent,
            mNodeUUID,
            mMessage->transactionUUID(),
            false,
            ConfirmationMessage::ContractorBanned);

        return resultDone();
    }

    auto previousTL = mTrustLines->trustLineReadOnly(mMessage->senderUUID);
    TrustLinesManager::TrustLineOperationResult kOperationResult;

    try {
        // note: io transaction would commit automatically on destructor call.
        // there is no need to call commit manually.
        kOperationResult = mTrustLines->setIncoming(
            mMessage->senderUUID,
            mMessage->amount());

        mTrustLines->setTrustLineState(
            mMessage->senderUUID,
            TrustLine::AuditPending,
            ioTransaction);

        switch (kOperationResult) {
        case TrustLinesManager::TrustLineOperationResult::Opened: {
            populateHistory(ioTransaction, TrustLineRecord::Accepting);
            mTopologyCacheManager->resetInitiatorCache();
            mMaxFlowCacheManager->clearCashes();
            info() << "Incoming trust line from the node " << mMessage->senderUUID
                   << " has been successfully initialised with " << mMessage->amount();

            if (mSenderIsGateway) {
                mTrustLines->setContractorAsGateway(
                    ioTransaction,
                    mMessage->senderUUID,
                    true);
                info() << "Incoming trust line was opened from gateway";
            }
            break;
        }

        case TrustLinesManager::TrustLineOperationResult::Updated: {
            populateHistory(ioTransaction, TrustLineRecord::Updating);
            mTopologyCacheManager->resetInitiatorCache();
            mMaxFlowCacheManager->clearCashes();
            info() << "Incoming trust line from the node " << mMessage->senderUUID
                   << " has been successfully set to " << mMessage->amount();
            break;
        }

        case TrustLinesManager::TrustLineOperationResult::Closed: {
            populateHistory(ioTransaction, TrustLineRecord::Rejecting);
            // remove this TL from Topology TrustLines Manager
            mTopologyTrustLinesManager->addTrustLine(
                make_shared<TopologyTrustLine>(
                    mNodeUUID,
                    mMessage->senderUUID,
                    make_shared<const TrustLineAmount>(0)));
            mTopologyCacheManager->resetInitiatorCache();
            mMaxFlowCacheManager->clearCashes();
            info() << "Incoming trust line from the node " << mMessage->senderUUID
                   << " has been successfully closed.";
            break;
        }

        case TrustLinesManager::TrustLineOperationResult::NoChanges: {
            // It is possible, that set trust line request will arrive twice or more times,
            // but only first processed update must be written to the trust lines history.
            info() << "Incoming trust line from the node " << mMessage->senderUUID
                   << " has not been changed.";
            break;
        }
        }

        // Sending confirmation back.
        sendMessage<TrustLineConfirmationMessage>(
            mMessage->senderUUID,
            mEquivalent,
            mNodeUUID,
            mMessage->transactionUUID(),
            mIAmGateway,
            ConfirmationMessage::OK);

        if (mSubsystemsController->isWriteVisualResults()) {
            if (kOperationResult == TrustLinesManager::TrustLineOperationResult::Opened) {
                stringstream s;
                s << VisualResult::IncomingTrustLineOpen << kTokensSeparator
                  << microsecondsSinceUnixEpoch() << kTokensSeparator
                  << currentTransactionUUID() << kTokensSeparator
                  << mMessage->senderUUID << kCommandsSeparator;
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
                  << mMessage->senderUUID << kCommandsSeparator;
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

        info() << "Wait for audit";
        return resultDone();

    } catch (ValueError &) {
        warning() << "Attempt to set incoming trust line from the node " << mMessage->senderUUID << " failed. "
               << "Cannot open TL with zero amount.";
        sendMessage<TrustLineConfirmationMessage>(
            mMessage->senderUUID,
            mEquivalent,
            mNodeUUID,
            mMessage->transactionUUID(),
            false,
            ConfirmationMessage::ErrorShouldBeRemovedFromQueue);
        return resultDone();

    } catch (IOError &e) {
        ioTransaction->rollback();
        mTrustLines->setIncoming(
            mMessage->senderUUID,
            previousTL->incomingTrustAmount());
        mTrustLines->setTrustLineState(
            mMessage->senderUUID,
            previousTL->state());
        sendMessage<TrustLineConfirmationMessage>(
            mMessage->senderUUID,
            mEquivalent,
            mNodeUUID,
            mMessage->transactionUUID(),
            false,
            ConfirmationMessage::ErrorShouldBeRemovedFromQueue);
        warning() << "Attempt to set incoming trust line to the node " << mMessage->senderUUID << " failed. "
               << "IO transaction can't be completed. "
               << "Details are: " << e.what();

        // Rethrowing the exception,
        // because the TA can't finish properly and no result may be returned.
        throw e;
    }
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
        mMessage->senderUUID,
        mMessage->amount());

    ioTransaction->historyStorage()->saveTrustLineRecord(
        record,
        mEquivalent);
#endif
}
