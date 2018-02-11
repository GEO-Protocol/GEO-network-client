#include "GatewayNotificationReceiverTransaction.h"

GatewayNotificationReceiverTransaction::GatewayNotificationReceiverTransaction(
    const NodeUUID &nodeUUID,
    GatewayNotificationMessage::Shared message,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    Logger &logger) :

    BaseTransaction(
        BaseTransaction::TransactionType::GatewayNotificationReceiverType,
        message->transactionUUID(),
        nodeUUID,
        logger),
    mMessage(message),
    mTrustLineManager(manager),
    mStorageHandler(storageHandler)
{}

TransactionResult::SharedConst GatewayNotificationReceiverTransaction::run()
{
    if (!mTrustLineManager->isNeighbor(mMessage->senderUUID)) {
        warning() << "Sender " << mMessage->senderUUID << " is not neighbor of current node";
        return resultDone();
    }
    auto ioTransaction = mStorageHandler->beginTransaction();
    try {
        // note: io transaction would commit automatically on destructor call.
        // there is no need to call commit manually.
        mTrustLineManager->setContractorAsGateway(
            ioTransaction,
            mMessage->senderUUID,
            mMessage->nodeState() == GatewayNotificationMessage::Gateway);
        info() << "Node " << mMessage->senderUUID << " inform that it is gateway "
               << (mMessage->nodeState() == GatewayNotificationMessage::Gateway);
    } catch (NotFoundError &e) {
        ioTransaction->rollback();
        warning() << "Attempt to set contractor " << mMessage->senderUUID << " as gateway failed. "
               << "Details are: " << e.what();
        return resultDone();
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
        currentNodeUUID(),
        mMessage->transactionUUID());
    return resultDone();
}

const string GatewayNotificationReceiverTransaction::logHeader() const
{
    stringstream s;
    s << "[GatewayNotificationReceiverTA: " << currentTransactionUUID() << "]";
    return s.str();
}
