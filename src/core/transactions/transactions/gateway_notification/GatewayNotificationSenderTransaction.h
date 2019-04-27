#ifndef GEO_NETWORK_CLIENT_GATEWAYNOTIFICATIONSENDERTRANSACTION_H
#define GEO_NETWORK_CLIENT_GATEWAYNOTIFICATIONSENDERTRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../contractors/ContractorsManager.h"
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
        ContractorsManager *contractorsManager,
        EquivalentsSubsystemsRouter *equivalentsSubsystemsRouter,
        EquivalentsCyclesSubsystemsRouter *equivalentsCyclesSubsystemsRouter,
        TailManager *tailManager,
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
    static const uint16_t kCountNeighborsPerOneStep = 20;
    static const uint32_t kHoursBetweenSteps = 0;
    static const uint32_t kMinutesBetweenSteps = 0;
    static const uint32_t kSecondsBetweenSteps = 30;
    static const uint32_t kHoursMax = 2;
    static const uint32_t kMinutesMax = 0;
    static const uint32_t kSecondsMax = 0;

    static Duration& kMaxDurationBetweenSteps() {
        static auto duration = Duration(
            kHoursBetweenSteps,
            kMinutesBetweenSteps,
            kSecondsBetweenSteps);
        return duration;
    }

    static Duration& kMaxTransactionDuration() {
        static auto duration = Duration(
            kHoursMax,
            kMinutesMax,
            kSecondsMax);
        return duration;
    }

private:
    ContractorsManager *mContractorsManager;
    EquivalentsSubsystemsRouter *mEquivalentsSubsystemsRouter;
    EquivalentsCyclesSubsystemsRouter *mEquivalentsCyclesSubsystemsRouter;
    set<ContractorID> allNeighborsRequestShouldBeSend;
    set<ContractorID> allNeighborsRequestAlreadySent;
    set<ContractorID> allNeighborsResponseReceive;
    vector<SerializedEquivalent> mGatewaysEquivalents;
    DateTime mTransactionStarted;
    DateTime mPreviousStepStarted;
    TailManager *mTailManager;
};


#endif //GEO_NETWORK_CLIENT_GATEWAYNOTIFICATIONSENDERTRANSACTION_H
