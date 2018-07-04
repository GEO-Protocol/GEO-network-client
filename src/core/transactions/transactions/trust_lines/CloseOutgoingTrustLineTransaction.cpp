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
        BaseTransaction::CloseOutgoingTrustLineTransactionType,
        message->transactionUUID(),
        nodeUUID,
        message->equivalent(),
        logger),
    mMessage(message),
    mTrustLines(manager),
    mStorageHandler(storageHandler),
    mTopologyCacheManager(topologyCacheManager),
    mMaxFlowCacheManager(maxFlowCacheManager)
{}

TransactionResult::SharedConst CloseOutgoingTrustLineTransaction::run()
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
    TrustLine::ConstShared previousTL = mTrustLines->trustLineReadOnly(mMessage->senderUUID);
    try {
        // note: io transaction would commit automatically on destructor call.
        // there is no need to call commit manually.
        mTrustLines->setTrustLineState(
            mMessage->senderUUID,
            TrustLine::AuditPending,
            ioTransaction);

        mTrustLines->closeOutgoing(
            mMessage->senderUUID);

        populateHistory(ioTransaction, TrustLineRecord::RejectingOutgoing);
        mTopologyCacheManager->resetInitiatorCache();
        mMaxFlowCacheManager->clearCashes();
        info() << "Outgoing trust line to the node " << mMessage->senderUUID
               << " has been successfully closed by remote node.";

        // Sending confirmation back.
        sendMessage<TrustLineConfirmationMessage>(
            mMessage->senderUUID,
            mEquivalent,
            mNodeUUID,
            mMessage->transactionUUID(),
            false,
            ConfirmationMessage::OK);

        info() << "Wait for audit";
        return resultDone();

    } catch (IOError &e) {
        ioTransaction->rollback();
        // return closed TL
        mTrustLines->setOutgoing(
            mMessage->senderUUID,
            previousTL->outgoingTrustAmount());
        mTrustLines->setTrustLineState(
            mMessage->senderUUID,
            previousTL->state());
        warning() << "Attempt to close outgoing trust line to the node " << mMessage->senderUUID << " failed. "
               << "IO transaction can't be completed. "
               << "Details are: " << e.what();
        sendMessage<TrustLineConfirmationMessage>(
            mMessage->senderUUID,
            mEquivalent,
            mNodeUUID,
            mMessage->transactionUUID(),
            false,
            ConfirmationMessage::ErrorShouldBeRemovedFromQueue);

        // Rethrowing the exception,
        // because the TA can't finish properly and no result may be returned.
        throw e;
    }
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
        mMessage->senderUUID);

    ioTransaction->historyStorage()->saveTrustLineRecord(
        record,
        mEquivalent);
#endif
}
