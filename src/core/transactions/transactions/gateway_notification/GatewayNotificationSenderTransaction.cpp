#include "GatewayNotificationSenderTransaction.h"

GatewayNotificationSenderTransaction::GatewayNotificationSenderTransaction(
    const NodeUUID &nodeUUID,
    EquivalentsSubsystemsRouter *equivalentsSubsystemsRouter,
    EquivalentsCyclesSubsystemsRouter *equivalentsCyclesSubsystemsRouter,
    Logger &logger) :

    BaseTransaction(
        BaseTransaction::TransactionType::GatewayNotificationSenderType,
        nodeUUID,
        0,
        logger),
    mEquivalentsSubsystemsRouter(equivalentsSubsystemsRouter),
    mEquivalentsCyclesSubsystemsRouter(equivalentsCyclesSubsystemsRouter),
    mTransactionStarted(utc_now())
{}

TransactionResult::SharedConst GatewayNotificationSenderTransaction::run()
{
    switch (mStep) {
        case Stages::GatewayNotificationStage:
            return sendGatewayNotification();

        case Stages::UpdateRoutingTableStage:
            return processRoutingTablesResponse();

        default:
            error() << "invalid transaction step " << mStep;
            return resultDone();
    }
}

TransactionResult::SharedConst GatewayNotificationSenderTransaction::sendGatewayNotification()
{
    for (const auto &equivalent : mEquivalentsSubsystemsRouter->equivalents()) {
        if (mEquivalentsSubsystemsRouter->iAmGateway(equivalent)) {
            mGatewaysEquivalents.push_back(
                equivalent);
        }
        for (const auto &neighbor : mEquivalentsSubsystemsRouter->trustLinesManager(equivalent)->firstLevelNeighbors()) {
            allNeighborsRequestShouldBeSend.insert(
                neighbor);
        }
    }
    uint16_t cntRequestedNeighbors = 0;
    for (const auto &neighbor : allNeighborsRequestShouldBeSend) {
        info() << "Send Gateway notification to node " << neighbor;
        sendMessage<GatewayNotificationMessage>(
            neighbor,
            currentNodeUUID(),
            currentTransactionUUID(),
            mGatewaysEquivalents);
        allNeighborsRequestAlreadySent.insert(neighbor);
        cntRequestedNeighbors++;
        if (cntRequestedNeighbors >= kCountNeighborsPerOneStep) {
            break;
        }
    }
    mEquivalentsCyclesSubsystemsRouter->clearRoutingTables();
    mPreviousStepStarted = utc_now();
    mStep = UpdateRoutingTableStage;
    return resultAwakeAfterMilliseconds(
        kCollectingRoutingTablesMilliseconds);
}

TransactionResult::SharedConst GatewayNotificationSenderTransaction::processRoutingTablesResponse()
{
    while (!mContext.empty()) {
        if (mContext.at(0)->typeID() == Message::RoutingTableResponse) {
            const auto kMessage = popNextMessage<RoutingTableResponseMessage>();
            for (const auto &equivalentAndNeighbors : kMessage->neighborsByEquivalents()) {
                allNeighborsResponseReceive.insert(kMessage->senderUUID);
                try {
                    auto trustLinesManager = mEquivalentsSubsystemsRouter->trustLinesManager(
                            equivalentAndNeighbors.first);
                    auto routingTablesManager = mEquivalentsCyclesSubsystemsRouter->routingTableManager(
                            equivalentAndNeighbors.first);
                    if(!trustLinesManager->isNeighbor(kMessage->senderUUID)){
                        warning() << "Node " << kMessage->senderUUID << " is not a neighbor";
                        continue;
                    }
                    routingTablesManager->updateMapAddSeveralNeighbors(
                        kMessage->senderUUID,
                        equivalentAndNeighbors.second);
                } catch (NotFoundError &e) {
                    warning() << "There is no subsystems for equivalent " << equivalentAndNeighbors.first;
                    continue;
                }
            }
        } else {
            warning() << "Invalid message type in context during processRoutingTablesResponse " << mContext.at(0)->typeID();
            mContext.pop_front();
        }
    }

    if (utc_now() - mTransactionStarted > kMaxTransactionDuration()) {
        warning() << "Not all nodes send response, but time is out.";
        return resultDone();
    }

    if (allNeighborsResponseReceive.size() < allNeighborsRequestAlreadySent.size()) {
        if (utc_now() - mPreviousStepStarted < kMaxDurationBetweenSteps()) {
            return resultAwakeAfterMilliseconds(
                kCollectingRoutingTablesMilliseconds);
        }
    }

    if (allNeighborsResponseReceive.size() < allNeighborsRequestShouldBeSend.size()) {
        info() << "Not all nodes send response";
        uint16_t cntRequestedNeighbors = 0;
        for (const auto &neighbor : allNeighborsRequestShouldBeSend) {
            if (allNeighborsRequestAlreadySent.count(neighbor) != 0) {
                continue;
            }
            info() << "Send Gateway notification to node " << neighbor;
            sendMessage<GatewayNotificationMessage>(
                neighbor,
                currentNodeUUID(),
                currentTransactionUUID(),
                mGatewaysEquivalents);
            allNeighborsRequestAlreadySent.insert(neighbor);
            cntRequestedNeighbors++;
            if (cntRequestedNeighbors >= kCountNeighborsPerOneStep) {
                break;
            }
        }
        mPreviousStepStarted = utc_now();
        return resultAwakeAfterMilliseconds(
            kCollectingRoutingTablesMilliseconds);
    }

    info() << "All data processed";
    return resultDone();
}

const string GatewayNotificationSenderTransaction::logHeader() const
{
    stringstream s;
    s << "[GatewayNotificationSenderTA: " << currentTransactionUUID() << " " << mEquivalent << "]";
    return s.str();
}
