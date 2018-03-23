#include "GatewayNotificationSenderTransaction.h"

GatewayNotificationSenderTransaction::GatewayNotificationSenderTransaction(
    const NodeUUID &nodeUUID,
    EquivalentsSubsystemsRouter *equivalentsSubsystemsRouter,
    Logger &logger) :

    BaseTransaction(
        BaseTransaction::TransactionType::GatewayNotificationSenderType,
        nodeUUID,
        0,
        logger),
    mEquivalentsSubsystemsRouter(equivalentsSubsystemsRouter)
{}

TransactionResult::SharedConst GatewayNotificationSenderTransaction::run()
{
    set<NodeUUID> allNeighbors;
    vector<SerializedEquivalent> gatewaysEquivalents;
    for (const auto &equivalent : mEquivalentsSubsystemsRouter->equivalents()) {
        if (mEquivalentsSubsystemsRouter->iAmGateway(equivalent)) {
            gatewaysEquivalents.push_back(
                equivalent);
        }
        for (const auto &neighbor : mEquivalentsSubsystemsRouter->trustLinesManager(equivalent)->firstLevelNeighbors()) {
            allNeighbors.insert(
                neighbor);
        }
    }
    for (const auto &neighbor : allNeighbors) {
        sendMessage<GatewayNotificationMessage>(
            neighbor,
            currentNodeUUID(),
            currentTransactionUUID(),
            gatewaysEquivalents);
    }
    return resultDone();
}

const string GatewayNotificationSenderTransaction::logHeader() const
{
    stringstream s;
    s << "[GatewayNotificationSenderTA: " << currentTransactionUUID() << " " << mEquivalent << "]";
    return s.str();
}
