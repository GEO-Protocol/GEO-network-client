#include "SetIncomingTrustLineTransaction.h"


SetIncomingTrustLineTransaction::SetIncomingTrustLineTransaction(
    const NodeUUID &nodeUUID,
    SetIncomingTrustLineMessage::Shared message,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    MaxFlowCalculationCacheManager *maxFlowCalculationCacheManager,
    bool iAmGateway,
    Logger &logger)
    noexcept:

    BaseTransaction(
        BaseTransaction::SetIncomingTrustLineTransaction,
        nodeUUID,
        logger),
    mMessage(message),
    mTrustLines(manager),
    mStorageHandler(storageHandler),
    mMaxFlowCalculationCacheManager(maxFlowCalculationCacheManager),
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
    try {
        // note: io transaction would commit automatically on destructor call.
        // there is no need to call commit manually.
        const auto kOperationResult = mTrustLines->setIncoming(
            ioTransaction,
            kContractor,
            mMessage->amount());

        switch (kOperationResult) {
        case TrustLinesManager::TrustLineOperationResult::Opened: {
            populateHistory(ioTransaction, TrustLineRecord::Accepting);
            mMaxFlowCalculationCacheManager->resetInitiatorCache();
            info() << "Incoming trust line from the node " << kContractor
                   << " has been successfully initialised with " << mMessage->amount();

            if (mIAmGateway) {
                // Notifying remote node that current node is gateway.
                // Network communicator knows, that this message must be forced to be delivered,
                // so the TA itself might finish without any response from the remote node.
                sendMessage<GatewayNotificationMessage>(
                    kContractor,
                    currentNodeUUID(),
                    currentTransactionUUID());
            }

            break;
        }

        case TrustLinesManager::TrustLineOperationResult::Updated: {
            populateHistory(ioTransaction, TrustLineRecord::Updating);
            mMaxFlowCalculationCacheManager->resetInitiatorCache();
            info() << "Incoming trust line from the node " << kContractor
                   << " has been successfully set to " << mMessage->amount();
            break;
        }

        case TrustLinesManager::TrustLineOperationResult::Closed: {
            populateHistory(ioTransaction, TrustLineRecord::Rejecting);
            mMaxFlowCalculationCacheManager->resetInitiatorCache();
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
        return resultDone();

    } catch (NotFoundError &e) {
        ioTransaction->rollback();
        info() << "Attempt to update incoming trust line from the node " << kContractor << " failed. "
               << "Details are: " << e.what();
        return resultDone();

    } catch (IOError &e) {
        ioTransaction->rollback();
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
