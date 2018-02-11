#include "SetIncomingTrustLineTransaction.h"


SetIncomingTrustLineTransaction::SetIncomingTrustLineTransaction(
    const NodeUUID &nodeUUID,
    SetIncomingTrustLineMessage::Shared message,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    MaxFlowCalculationCacheManager *maxFlowCalculationCacheManager,
    MaxFlowCalculationNodeCacheManager *maxFlowCalculationNodeCacheManager,
    bool iAmGateway,
    Logger &logger)
    noexcept:

    BaseTransaction(
        BaseTransaction::SetIncomingTrustLineTransaction,
        message->transactionUUID(),
        nodeUUID,
        logger),
    mMessage(message),
    mTrustLines(manager),
    mStorageHandler(storageHandler),
    mMaxFlowCalculationCacheManager(maxFlowCalculationCacheManager),
    mMaxFlowCalculationNodeCacheManager(maxFlowCalculationNodeCacheManager),
    mIAmGateway(iAmGateway)
{}

TransactionResult::SharedConst SetIncomingTrustLineTransaction::run()
{
    const auto kContractor = mMessage->senderUUID;

    if (kContractor == mNodeUUID) {
        info() << "Attempt to launch transaction against itself was prevented.";
        return resultDone();
    }

    // Trust line must be created (or updated) in the internal storage.
    // Also, history record must be written about this operation.
    // Both writes must be done atomically, so the IO transaction is used.
    auto ioTransaction = mStorageHandler->beginTransaction();

    // if contractor in black list we should reject operation with TL
    if (ioTransaction->blackListHandler()->checkIfNodeExists(kContractor)) {
        info() << "Contractor " << kContractor << " is in black list. Transaction rejected";
        sendMessage<ConfirmationMessage>(
            mMessage->senderUUID,
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
            mMaxFlowCalculationCacheManager->resetInitiatorCache();
            mMaxFlowCalculationNodeCacheManager->clearCashes();
            info() << "Incoming trust line from the node " << kContractor
                   << " has been successfully initialised with " << mMessage->amount();

            if (mIAmGateway) {
                // Notifying remote node that current node is gateway.
                // Network communicator knows, that this message must be forced to be delivered,
                // so the TA itself might finish without any response from the remote node.
                sendMessage<GatewayNotificationMessage>(
                    kContractor,
                    currentNodeUUID(),
                    currentTransactionUUID(),
                    GatewayNotificationMessage::Gateway);
            }

            break;
        }

        case TrustLinesManager::TrustLineOperationResult::Updated: {
            populateHistory(ioTransaction, TrustLineRecord::Updating);
            mMaxFlowCalculationCacheManager->resetInitiatorCache();
            mMaxFlowCalculationNodeCacheManager->clearCashes();
            info() << "Incoming trust line from the node " << kContractor
                   << " has been successfully set to " << mMessage->amount();
            break;
        }

        case TrustLinesManager::TrustLineOperationResult::Closed: {
            populateHistory(ioTransaction, TrustLineRecord::Rejecting);
            mMaxFlowCalculationCacheManager->resetInitiatorCache();
            mMaxFlowCalculationNodeCacheManager->clearCashes();
            info() << "Incoming trust line from the node " << kContractor
                   << " has been successfully closed.";
            break;
        }

        case TrustLinesManager::TrustLineOperationResult::NoChanges: {
            // It is possible, that set trust line request will arrive twice or more times,
            // but only first processed update must be written to the trust lines history.
            break;
        }
        }

        // Sending confirmation back.
        sendMessage<ConfirmationMessage>(
            mMessage->senderUUID,
            mNodeUUID,
            mMessage->transactionUUID());

        return resultDone();

    } catch (ValueError &) {
        ioTransaction->rollback();
        info() << "Attempt to set incoming trust line from the node " << kContractor << " failed. "
               << "Cannot open trustline with zero amount.";
        sendMessage<ConfirmationMessage>(
            mMessage->senderUUID,
            mNodeUUID,
            mMessage->transactionUUID(),
            ConfirmationMessage::ErrorShouldBeRemovedFromQueue);
        return resultDone();

    } catch (NotFoundError &e) {
        ioTransaction->rollback();
        info() << "Attempt to update incoming trust line from the node " << kContractor << " failed. "
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
        info() << "Attempt to set incoming trust line to the node " << kContractor << " failed. "
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
    s << "[SetIncomingTrustLineTA: " << currentTransactionUUID() << "]";
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

    ioTransaction->historyStorage()->saveTrustLineRecord(record);
#endif
}
