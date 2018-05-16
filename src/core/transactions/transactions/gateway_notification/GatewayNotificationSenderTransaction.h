#ifndef GEO_NETWORK_CLIENT_GATEWAYNOTIFICATIONSENDERTRANSACTION_H
#define GEO_NETWORK_CLIENT_GATEWAYNOTIFICATIONSENDERTRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../equivalents/EquivalentsSubsystemsRouter.h"
#include "../../../equivalents/EquivalentsCyclesSubsystemsRouter.h"
#include "../../../network/messages/gateway_notification_and_routing_tables/GatewayNotificationMessage.h"
#include "../../../network/messages/gateway_notification_and_routing_tables/RoutingTableResponseMessage.h"

#include <set>

class GatewayNotificationSenderTransaction : public BaseTransaction {

public:
    typedef shared_ptr<GatewayNotificationSenderTransaction> Shared;

public:
    GatewayNotificationSenderTransaction(
        const NodeUUID &nodeUUID,
        EquivalentsSubsystemsRouter *equivalentsSubsystemsRouter,
        EquivalentsCyclesSubsystemsRouter *equivalentsCyclesSubsystemsRouter,
        Logger &logger);

    TransactionResult::SharedConst run();

protected:
    enum Stages {
        GatewayNotificationStage = 1,
        UpdateRoutingTableStage
    };

protected:
    TransactionResult::SharedConst sendGatewayNotification();

    TransactionResult::SharedConst processRoutingTablesResponse();

    const string logHeader() const;

private:
    static const uint32_t kCollectingRoutingTablesMilliseconds = 6000;
    static const uint16_t kCountNeighborsPerOneStep = 15;
    static const uint32_t kHoursBetweenSteps = 0;
    static const uint32_t kMinutesBetweenSteps = 1;
    static const uint32_t kSecondsBetweenSteps = 0;

    static Duration& kMaxDurationBetweenSteps() {
        static auto duration = Duration(
            kHoursBetweenSteps,
            kMinutesBetweenSteps,
            kSecondsBetweenSteps);
        return duration;
    }

private:
    EquivalentsSubsystemsRouter *mEquivalentsSubsystemsRouter;
    EquivalentsCyclesSubsystemsRouter *mEquivalentsCyclesSubsystemsRouter;
    set<NodeUUID> allNeighborsRequestShouldBeSend;
    set<NodeUUID> allNeighborsRequestAlreadySent;
    set<NodeUUID> allNeighborsResponseReceive;
    vector<SerializedEquivalent> mGatewaysEquivalents;
    DateTime mPreviousStepStarted;
};


#endif //GEO_NETWORK_CLIENT_GATEWAYNOTIFICATIONSENDERTRANSACTION_H
