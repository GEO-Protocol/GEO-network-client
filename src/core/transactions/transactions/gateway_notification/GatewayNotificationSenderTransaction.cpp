#include "GatewayNotificationSenderTransaction.h"

GatewayNotificationSenderTransaction::GatewayNotificationSenderTransaction(
    const NodeUUID &nodeUUID,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    bool iAmGateway,
    Logger &logger) :

    BaseTransaction(
        BaseTransaction::TransactionType::GatewayNotificationSenderType,
        nodeUUID,
        logger),

    mTrustLineManager(manager),
    mStorageHandler(storageHandler),
    mIAmGateway(iAmGateway)
{}

TransactionResult::SharedConst GatewayNotificationSenderTransaction::run()
{
    bool wasGatewayOnPreviousSession = false;
    auto ioTransaction = mStorageHandler->beginTransaction();

    try {
        ioTransaction->nodeFeaturesHandler()->featureValue(kGatewayFeatureName);
        wasGatewayOnPreviousSession = true;
    } catch (NotFoundError) {}

    if (mIAmGateway) {
        if (!wasGatewayOnPreviousSession) {
            info() << "Current node was't recent gateway, but now is";
            ioTransaction->nodeFeaturesHandler()->saveRecord(kGatewayFeatureName);
            for (const auto &neighbor : mTrustLineManager->rt1()) {
                // Notifying remote node that current node is gateway.
                // Network communicator knows, that this message must be forced to be delivered,
                // so the TA itself might finish without any response from the remote node.
                info() << "Send message that I am gateway to " << neighbor;
                sendMessage<GatewayNotificationMessage>(
                    neighbor,
                    currentNodeUUID(),
                    currentTransactionUUID(),
                    GatewayNotificationMessage::Gateway);
            }
        }
    } else {
        if (wasGatewayOnPreviousSession) {
            info() << "Current node was gateway, but now isn't";
            ioTransaction->nodeFeaturesHandler()->deleteRecord(kGatewayFeatureName);
            for (const auto &neighbor : mTrustLineManager->rt1()) {
                // Notifying remote node that current node is not gateway.
                // Network communicator knows, that this message must be forced to be delivered,
                // so the TA itself might finish without any response from the remote node.
                info() << "Send message that I am common node to " << neighbor;
                sendMessage<GatewayNotificationMessage>(
                    neighbor,
                    currentNodeUUID(),
                    currentTransactionUUID(),
                    GatewayNotificationMessage::Common);
            }
        }
    }
    return resultDone();
}

const string GatewayNotificationSenderTransaction::logHeader() const
{
    stringstream s;
    s << "[GatewayNotificationSenderTA: " << currentTransactionUUID() << "]";
    return s.str();
}
