#include "CloseOutgoingTrustLineTransaction.h"

CloseOutgoingTrustLineTransaction::CloseOutgoingTrustLineTransaction(
    const NodeUUID &nodeUUID,
    CloseOutgoingTrustLineMessage::Shared message,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    TopologyCacheManager *topologyCacheManager,
    MaxFlowCacheManager *maxFlowCacheManager,
    Logger &logger)
    noexcept:

    BaseTransaction(
        BaseTransaction::CloseOutgoingTrustLineTransaction,
        message->transactionUUID(),
        nodeUUID,
        logger),
    mMessage(message),
    mTrustLines(manager),
    mStorageHandler(storageHandler),
    mTopologyCacheManager(topologyCacheManager),
    mMaxFlowCacheManager(maxFlowCacheManager)
{}

TransactionResult::SharedConst CloseOutgoingTrustLineTransaction::run()
{
    const auto kContractor = mMessage->senderUUID;

    if (kContractor == mNodeUUID) {
        warning() << "Attempt to launch transaction against itself was prevented.";
        return resultDone();
    }

    // Trust line must be created (or updated) in the internal storage.
    // Also, history record must be written about this operation.
    // Both writes must be done atomically, so the IO transaction is used.
    auto ioTransaction = mStorageHandler->beginTransaction();
    TrustLine::ConstShared previousTL = nullptr;
    try {
        previousTL = mTrustLines->trustLineReadOnly(mMessage->senderUUID);
        // note: io transaction would commit automatically on destructor call.
        // there is no need to call commit manually.
        mTrustLines->closeOutgoing(
            ioTransaction,
            kContractor);

        populateHistory(ioTransaction, TrustLineRecord::RejectingOutgoing);
        mTopologyCacheManager->resetInitiatorCache();
        mMaxFlowCacheManager->clearCashes();
        info() << "Outgoing trust line to the node " << kContractor
               << " has been successfully closed by remote node.";

        // Sending confirmation back.
        sendMessage<ConfirmationMessage>(
            mMessage->senderUUID,
            mNodeUUID,
            mMessage->transactionUUID());

        return resultDone();

    } catch (NotFoundError &e) {
        ioTransaction->rollback();
        warning() << "Attempt to close outgoing trust line to the node " << kContractor << " failed. "
               << "Details are: " << e.what();
        sendMessage<ConfirmationMessage>(
            mMessage->senderUUID,
            mNodeUUID,
            mMessage->transactionUUID(),
            ConfirmationMessage::ErrorShouldBeRemovedFromQueue);
        return resultDone();

    } catch (IOError &e) {
        ioTransaction->rollback();
        // return closed TL
        auto trustLine = make_shared<TrustLine>(
            mMessage->senderUUID,
            previousTL->incomingTrustAmount(),
            previousTL->outgoingTrustAmount(),
            previousTL->balance(),
            previousTL->isContractorGateway());
        warning() << "Attempt to close outgoing trust line to the node " << kContractor << " failed. "
               << "IO transaction can't be completed. "
               << "Details are: " << e.what();

        // Rethrowing the exception,
        // because the TA can't finish propely and no result may be returned.
        throw e;
    }
}

TransactionResult::SharedConst CloseOutgoingTrustLineTransaction::resultDone()
{
    return make_shared<TransactionResult>(
        TransactionState::exit());
}

const string CloseOutgoingTrustLineTransaction::logHeader() const
noexcept
{
    stringstream s;
    s << "[CloseOutgoingTrustLineTA: " << currentTransactionUUID() << "]";
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
        mMessage->senderUUID);

    ioTransaction->historyStorage()->saveTrustLineRecord(record);
#endif
}
