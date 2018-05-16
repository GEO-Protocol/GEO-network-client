#ifndef GEO_NETWORK_CLIENT_GATEWAYNOTIFICATIONRECEIVERTRANSACTION_H
#define GEO_NETWORK_CLIENT_GATEWAYNOTIFICATIONRECEIVERTRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../equivalents/EquivalentsSubsystemsRouter.h"
#include "../../../io/storage/StorageHandler.h"
#include "../../../network/messages/gateway_notification_and_routing_tables/GatewayNotificationMessage.h"
#include "../../../network/messages/base/transaction/ConfirmationMessage.h"
#include "../../../network/messages/gateway_notification_and_routing_tables/RoutingTableResponseMessage.h"

class GatewayNotificationReceiverTransaction : public BaseTransaction {

public:
    typedef shared_ptr<GatewayNotificationReceiverTransaction> Shared;

public:
    GatewayNotificationReceiverTransaction(
        const NodeUUID &nodeUUID,
        GatewayNotificationMessage::Shared message,
        EquivalentsSubsystemsRouter *equivalentsSubsystemsRouter,
        StorageHandler *storageHandler,
        Logger &logger);

    TransactionResult::SharedConst run();

protected:
    const string logHeader() const;

private:
    set<NodeUUID> getNeighborsForEquivalent(
        const SerializedEquivalent equivalent) const;

private:
    GatewayNotificationMessage::Shared mMessage;
    EquivalentsSubsystemsRouter *mEquivalentsSubsystemsRouter;
    StorageHandler *mStorageHandler;
};


#endif //GEO_NETWORK_CLIENT_GATEWAYNOTIFICATIONRECEIVERTRANSACTION_H
