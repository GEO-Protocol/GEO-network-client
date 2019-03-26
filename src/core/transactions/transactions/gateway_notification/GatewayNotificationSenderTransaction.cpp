#include "GatewayNotificationSenderTransaction.h"

GatewayNotificationSenderTransaction::GatewayNotificationSenderTransaction(
    ContractorsManager *contractorsManager,
    EquivalentsSubsystemsRouter *equivalentsSubsystemsRouter,
    EquivalentsCyclesSubsystemsRouter *equivalentsCyclesSubsystemsRouter,
    TailManager &tailManager,
    Logger &logger) :

    BaseTransaction(
        BaseTransaction::TransactionType::GatewayNotificationSenderType,
        0,
        logger),
    mContractorsManager(contractorsManager),
    mEquivalentsSubsystemsRouter(equivalentsSubsystemsRouter),
    mEquivalentsCyclesSubsystemsRouter(equivalentsCyclesSubsystemsRouter),
    mTransactionStarted(utc_now()),
    mTailManager(tailManager)
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
#ifdef DEBUG_LOG_ROUTING_TABLES_PROCESSING
        mEquivalentsCyclesSubsystemsRouter->routingTableManager(equivalent)->printRT();
#endif
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
            mContractorsManager->idOnContractorSide(neighbor),
            mTransactionUUID,
            mGatewaysEquivalents);
        allNeighborsRequestAlreadySent.insert(neighbor);
        cntRequestedNeighbors++;
        if (cntRequestedNeighbors >= kCountNeighborsPerOneStep) {
            break;
        }
    }
    mPreviousStepStarted = utc_now();
    mStep = UpdateRoutingTableStage;
    return resultAwakeAfterMilliseconds(
        kCollectingRoutingTablesMilliseconds);
}

TransactionResult::SharedConst GatewayNotificationSenderTransaction::processRoutingTablesResponse()
{
    /// Take messages from TailManager instead of BaseTransaction's 'mContext'
    auto &mContext = mTailManager.getRoutingTableTail();

    if (allNeighborsResponseReceive.empty()) {
        mEquivalentsCyclesSubsystemsRouter->clearRoutingTables();
    }
    while (!mContext.empty()) {
        if (mContext.at(0)->typeID() == Message::RoutingTableResponse) {
            const auto kMessage = popNextMessage<RoutingTableResponseMessage>(mContext);
            info() << "node " << kMessage->idOnReceiverSide << " send response";
            allNeighborsResponseReceive.insert(kMessage->idOnReceiverSide);
            for (const auto &equivalentAndNeighbors : kMessage->neighborsByEquivalents()) {
                try {
                    auto trustLinesManager = mEquivalentsSubsystemsRouter->trustLinesManager(
                        equivalentAndNeighbors.first);
                    auto routingTablesManager = mEquivalentsCyclesSubsystemsRouter->routingTableManager(
                        equivalentAndNeighbors.first);
                    if(!trustLinesManager->trustLineIsPresent(kMessage->idOnReceiverSide)){
                        warning() << "Node " << kMessage->idOnReceiverSide << " is not a neighbor on equivalent "
                                  << equivalentAndNeighbors.first;
                        continue;
                    }
                    routingTablesManager->updateMapAddSeveralNeighbors(
                        kMessage->idOnReceiverSide,
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

    if (allNeighborsResponseReceive.size() < allNeighborsRequestAlreadySent.size()) {
        if (utc_now() - mPreviousStepStarted < kMaxDurationBetweenSteps()) {
            return resultAwakeAfterMilliseconds(
                kCollectingRoutingTablesMilliseconds);
        }
    }

    mPreviousStepStarted = utc_now();
    if (allNeighborsRequestAlreadySent.size() < allNeighborsRequestShouldBeSend.size()) {
        uint16_t cntRequestedNeighbors = 0;
        for (const auto &neighbor : allNeighborsRequestShouldBeSend) {
            if (allNeighborsRequestAlreadySent.count(neighbor) != 0) {
                continue;
            }
            info() << "Send Gateway notification to node " << neighbor;
            sendMessage<GatewayNotificationMessage>(
                neighbor,
                mContractorsManager->idOnContractorSide(neighbor),
                mTransactionUUID,
                mGatewaysEquivalents);
            allNeighborsRequestAlreadySent.insert(neighbor);
            cntRequestedNeighbors++;
            if (cntRequestedNeighbors >= kCountNeighborsPerOneStep) {
                break;
            }
        }
        return resultAwakeAfterMilliseconds(
            kCollectingRoutingTablesMilliseconds);
    }

    if (utc_now() - mTransactionStarted > kMaxTransactionDuration()) {
        if (allNeighborsResponseReceive.size() < allNeighborsRequestShouldBeSend.size()) {
            warning() << "Not all nodes send response, but time is out.";
        } else {
            info() << "All data processed after time limit";
        }
        return resultDone();
    }

    if (allNeighborsResponseReceive.size() < allNeighborsRequestShouldBeSend.size()) {
        info() << "Not all nodes send response";
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
