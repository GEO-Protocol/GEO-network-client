#ifndef GEO_NETWORK_CLIENT_ROUTINGTABLEUPDATINGTRANSACTION_H
#define GEO_NETWORK_CLIENT_ROUTINGTABLEUPDATINGTRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../equivalents/EquivalentsSubsystemsRouter.h"
#include "../../../equivalents/EquivalentsCyclesSubsystemsRouter.h"
#include "../../../network/messages/gateway_notification_and_routing_tables/RoutingTableResponseMessage.h"

class RoutingTableUpdatingTransaction : public BaseTransaction {

public:
    typedef shared_ptr<RoutingTableUpdatingTransaction> Shared;

public:
    RoutingTableUpdatingTransaction(
        const NodeUUID &nodeUUID,
        RoutingTableResponseMessage::Shared message,
        EquivalentsSubsystemsRouter *equivalentsSubsystemsRouter,
        EquivalentsCyclesSubsystemsRouter *equivalentsCyclesSubsystemsRouter,
        Logger &logger);

    TransactionResult::SharedConst run();

protected:
    const string logHeader() const;

private:
    RoutingTableResponseMessage::Shared mMessage;
    EquivalentsSubsystemsRouter *mEquivalentsSubsystemsRouter;
    EquivalentsCyclesSubsystemsRouter *mEquivalentsCyclesSubsystemsRouter;
};


#endif //GEO_NETWORK_CLIENT_ROUTINGTABLEUPDATINGTRANSACTION_H
