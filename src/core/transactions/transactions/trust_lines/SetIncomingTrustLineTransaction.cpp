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
    const auto kContractor = mMessage->senderUUID;
    info() << "sender: " << mMessage->senderUUID;
    info() << "equivalent: " << mMessage->equivalent();

    if (kContractor == mNodeUUID) {
        warning() << "Attempt to launch transaction against itself was prevented.";
        return resultDone();
    }

    // Trust line must be created (or updated) in the internal storage.
    // Also, history record must be written about this operation.
    // Both writes must be done atomically, so the IO transaction is used.
    auto ioTransaction = mStorageHandler->beginTransaction();

    // if contractor in black list we should reject operation with TL
    if (ioTransaction->blackListHandler()->checkIfNodeExists(kContractor)) {
        warning() << "Contractor " << kContractor << " is in black list. Transaction rejected";
        sendMessage<ConfirmationMessage>(
            mMessage->senderUUID,
            mEquivalent,
            mNodeUUID,
            mMessage->transactionUUID(),
            ConfirmationMessage::ContractorBanned);

        return resultDone();
    }

    TrustLine::ConstShared previousTL = nullptr;
    try {
        previousTL = mTrustLines->trustLineReadOnly(mMessage->senderUUID);
    } catch (NotFoundError &e) {
        // Nothing actions, because TL will be created
    }

    TrustLinesManager::TrustLineOperationResult kOperationResult;

    try {
        // note: io transaction would commit automatically on destructor call.
        // there is no need to call commit manually.
        kOperationResult = mTrustLines->setIncoming(
            ioTransaction,
            kContractor,
            mMessage->amount());

        switch (kOperationResult) {
        case TrustLinesManager::TrustLineOperationResult::Opened: {
            populateHistory(ioTransaction, TrustLineRecord::Accepting);
            mTopologyCacheManager->resetInitiatorCache();
            mMaxFlowCacheManager->clearCashes();
            info() << "Incoming trust line from the node " << kContractor
                   << " has been successfully initialised with " << mMessage->amount();

            if (mSenderIsGateway) {
                mTrustLines->setContractorAsGateway(
                    ioTransaction,
                    mMessage->senderUUID,
                    true);
                info() << "Incoming trust line was opened from gateway";
            }

            if (mIAmGateway) {
                // Notifying remote node that current node is gateway.
                // Network communicator knows, that this message must be forced to be delivered,
                // so the TA itself might finish without any response from the remote node.
                sendMessage<GatewayNotificationOneEquivalentMessage>(
                    kContractor,
                    mEquivalent,
                    currentNodeUUID(),
                    currentTransactionUUID());
            }

            break;
        }

        case TrustLinesManager::TrustLineOperationResult::Updated: {
            populateHistory(ioTransaction, TrustLineRecord::Updating);
            mTopologyCacheManager->resetInitiatorCache();
            mMaxFlowCacheManager->clearCashes();
            info() << "Incoming trust line from the node " << kContractor
                   << " has been successfully set to " << mMessage->amount();
            break;
        }

        case TrustLinesManager::TrustLineOperationResult::Closed: {
            populateHistory(ioTransaction, TrustLineRecord::Rejecting);
            // remove this TL from Topology TrustLines Manager
            mTopologyTrustLinesManager->addTrustLine(
                make_shared<TopologyTrustLine>(
                    mNodeUUID,
                    kContractor,
                    make_shared<const TrustLineAmount>(0)));
            mTopologyCacheManager->resetInitiatorCache();
            mMaxFlowCacheManager->clearCashes();
            info() << "Incoming trust line from the node " << kContractor
                   << " has been successfully closed.";
            break;
        }

        case TrustLinesManager::TrustLineOperationResult::NoChanges: {
            // It is possible, that set trust line request will arrive twice or more times,
            // but only first processed update must be written to the trust lines history.
            info() << "Incoming trust line from the node " << kContractor
                   << " has not been changed.";
            break;
        }
        }

        // Sending confirmation back.
        sendMessage<ConfirmationMessage>(
            mMessage->senderUUID,
            mEquivalent,
            mNodeUUID,
            mMessage->transactionUUID());

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

        return resultDone();

    } catch (ValueError &) {
        ioTransaction->rollback();
        warning() << "Attempt to set incoming trust line from the node " << kContractor << " failed. "
               << "Cannot open trustline with zero amount.";
        sendMessage<ConfirmationMessage>(
            mMessage->senderUUID,
            mEquivalent,
            mNodeUUID,
            mMessage->transactionUUID(),
            ConfirmationMessage::ErrorShouldBeRemovedFromQueue);
        return resultDone();

    } catch (NotFoundError &e) {
        ioTransaction->rollback();
        warning() << "Attempt to update incoming trust line from the node " << kContractor << " failed. "
               << "Details are: " << e.what();
        return resultDone();

    } catch (IOError &e) {
        ioTransaction->rollback();
        switch (kOperationResult) {
            case TrustLinesManager::TrustLineOperationResult::Opened: {
                // remove created TL from TrustLinesManager
                mTrustLines->trustLines().erase(mMessage->senderUUID);
                break;
            }
            case TrustLinesManager::TrustLineOperationResult::Updated: {
                // Change outgoing TL amount to previous value
                mTrustLines->trustLines()[mMessage->senderUUID]->setOutgoingTrustAmount(
                    previousTL->outgoingTrustAmount());
                break;
            }
            case TrustLinesManager::TrustLineOperationResult::Closed: {
                // return closed TL
                auto trustLine = make_shared<TrustLine>(
                    mMessage->senderUUID,
                    previousTL->incomingTrustAmount(),
                    previousTL->outgoingTrustAmount(),
                    previousTL->balance(),
                    previousTL->isContractorGateway());
                mTrustLines->trustLines()[mMessage->senderUUID] = trustLine;
                break;
            }
        }
        warning() << "Attempt to set incoming trust line to the node " << kContractor << " failed. "
               << "IO transaction can't be completed. "
               << "Details are: " << e.what();

        // Rethrowing the exception,
        // because the TA can't finish propely and no result may be returned.
        throw e;
    }
}

TransactionResult::SharedConst SetIncomingTrustLineTransaction::resultDone()
{
    return make_shared<TransactionResult>(
        TransactionState::exit());
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
