#include "GatewayNotificationOneEquivalentReceiverTransaction.h"

GatewayNotificationOneEquivalentReceiverTransaction::GatewayNotificationOneEquivalentReceiverTransaction(
    const NodeUUID &nodeUUID,
    GatewayNotificationOneEquivalentMessage::Shared message,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    Logger &logger) :

    BaseTransaction(
        BaseTransaction::TransactionType::GatewayNotificationOneEquivalentReceiverType,
        message->transactionUUID(),
        nodeUUID,
        message->equivalent(),
        logger),
    mMessage(message),
    mTrustLineManager(manager),
    mStorageHandler(storageHandler)
{}

TransactionResult::SharedConst GatewayNotificationOneEquivalentReceiverTransaction::run()
{
    if (!mTrustLineManager->isNeighbor(mMessage->senderUUID)) {
        warning() << "Sender " << mMessage->senderUUID << " is not neighbor of current node";
        sendMessage<ConfirmationMessage>(
            mMessage->senderUUID,
            mEquivalent,
            currentNodeUUID(),
            mMessage->transactionUUID(),
            ConfirmationMessage::ErrorShouldBeRemovedFromQueue);
        return resultDone();
    }
    auto ioTransaction = mStorageHandler->beginTransaction();
    try {
        mTrustLineManager->setContractorAsGateway(
            ioTransaction,
            mMessage->senderUUID,
            true);
        info() << "Node " << mMessage->senderUUID << " inform that it is gateway";
    } catch (IOError &e) {
        ioTransaction->rollback();
        warning() << "Attempt to set to set contractor " << mMessage->senderUUID << " as gateway failed. "
                  << "IO transaction can't be completed. "
                  << "Details are: " << e.what();
        return resultDone();
    }

    debug() << "Send confirmation to node " << mMessage->senderUUID;
    sendMessage<ConfirmationMessage>(
        mMessage->senderUUID,
        mEquivalent,
        currentNodeUUID(),
        mMessage->transactionUUID());
    return resultDone();
}

const string GatewayNotificationOneEquivalentReceiverTransaction::logHeader() const
{
    stringstream s;
    s << "[GatewayNotificationOneEquivalentReceiverTA: " << currentTransactionUUID() << " " << mEquivalent << "]";
    return s.str();
}
