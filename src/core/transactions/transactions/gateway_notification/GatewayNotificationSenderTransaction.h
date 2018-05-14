#ifndef GEO_NETWORK_CLIENT_GATEWAYNOTIFICATIONSENDERTRANSACTION_H
#define GEO_NETWORK_CLIENT_GATEWAYNOTIFICATIONSENDERTRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../equivalents/EquivalentsSubsystemsRouter.h"
#include "../../../equivalents/EquivalentsCyclesSubsystemsRouter.h"
#include "../../../network/messages/gateway_notification/GatewayNotificationMessage.h"
#include "../../../network/messages/routing_table/RoutingTableResponseMessage.h"

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
    static const uint32_t kCollectingRoutingTablesMilliseconds = 5000;
    static const uint16_t kCountNeighborsPerOneStep = 10;
    static const uint32_t kSecondsBetweenSteps = 30;

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
