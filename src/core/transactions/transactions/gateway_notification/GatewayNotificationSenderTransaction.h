#ifndef GEO_NETWORK_CLIENT_GATEWAYNOTIFICATIONSENDERTRANSACTION_H
#define GEO_NETWORK_CLIENT_GATEWAYNOTIFICATIONSENDERTRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../equivalents/EquivalentsSubsystemsRouter.h"
#include "../../../network/messages/gateway_notification/GatewayNotificationMessage.h"

#include <set>

class GatewayNotificationSenderTransaction : public BaseTransaction {

public:
    typedef shared_ptr<GatewayNotificationSenderTransaction> Shared;

public:
    GatewayNotificationSenderTransaction(
        const NodeUUID &nodeUUID,
        EquivalentsSubsystemsRouter *equivalentsSubsystemsRouter,
        Logger &logger);

    TransactionResult::SharedConst run();

protected:
    const string logHeader() const;

private:
    EquivalentsSubsystemsRouter *mEquivalentsSubsystemsRouter;
};


#endif //GEO_NETWORK_CLIENT_GATEWAYNOTIFICATIONSENDERTRANSACTION_H
