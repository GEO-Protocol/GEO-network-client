#include "GatewayNotificationSenderTransaction.h"

GatewayNotificationSenderTransaction::GatewayNotificationSenderTransaction(
    const NodeUUID &nodeUUID,
    TrustLinesManager *manager,
    Logger &logger) :

    BaseTransaction(
        BaseTransaction::TransactionType::GatewayNotificationSenderType,
        nodeUUID,
        logger),

    mTrustLineManager(manager)
{}

TransactionResult::SharedConst GatewayNotificationSenderTransaction::run()
{
    for (const auto neighbor : mTrustLineManager->rt1()) {
        // Notifying remote node that current node is gateway.
        // Network communicator knows, that this message must be forced to be delivered,
        // so the TA itself might finish without any response from the remote node.
        sendMessage<GatewayNotificationMessage>(
            neighbor,
            currentNodeUUID(),
            currentTransactionUUID());
    }
    return resultDone();
}

const string GatewayNotificationSenderTransaction::logHeader() const
{
    stringstream s;
    s << "[GatewayNotificationSenderTA: " << currentTransactionUUID() << "]";
    return s.str();
}
