#ifndef GEO_NETWORK_CLIENT_GATEWAYNOTIFICATIONSENDERTRANSACTION_H
#define GEO_NETWORK_CLIENT_GATEWAYNOTIFICATIONSENDERTRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../io/storage/StorageHandler.h"
#include "../../../network/messages/gateway_notification/GatewayNotificationMessage.h"

class GatewayNotificationSenderTransaction : public BaseTransaction {

public:
    typedef shared_ptr<GatewayNotificationSenderTransaction> Shared;

public:
    GatewayNotificationSenderTransaction(
        const NodeUUID &nodeUUID,
        const SerializedEquivalent equivalent,
        TrustLinesManager *manager,
        StorageHandler *storageHandler,
        bool iAmGateway,
        Logger &logger);

    TransactionResult::SharedConst run();

protected:
    const string logHeader() const;

private:
    const string kGatewayFeatureName = "GATEWAY";

private:
    TrustLinesManager *mTrustLineManager;
    StorageHandler *mStorageHandler;
    bool mIAmGateway;
};


#endif //GEO_NETWORK_CLIENT_GATEWAYNOTIFICATIONSENDERTRANSACTION_H
