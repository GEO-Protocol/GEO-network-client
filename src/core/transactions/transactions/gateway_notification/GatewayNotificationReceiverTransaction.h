#ifndef GEO_NETWORK_CLIENT_GATEWAYNOTIFICATIONRECEIVERTRANSACTION_H
#define GEO_NETWORK_CLIENT_GATEWAYNOTIFICATIONRECEIVERTRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../io/storage/StorageHandler.h"
#include "../../../network/messages/gateway_notification/GatewayNotificationMessage.h"
#include "../../../network/messages/base/transaction/ConfirmationMessage.h"

class GatewayNotificationReceiverTransaction : public BaseTransaction {

public:
    typedef shared_ptr<GatewayNotificationReceiverTransaction> Shared;

public:
    GatewayNotificationReceiverTransaction(
        const NodeUUID &nodeUUID,
        GatewayNotificationMessage::Shared message,
        TrustLinesManager *manager,
        StorageHandler *storageHandler,
        Logger &logger);

    TransactionResult::SharedConst run();

protected:
    const string logHeader() const;

private:
    GatewayNotificationMessage::Shared mMessage;
    TrustLinesManager *mTrustLineManager;
    StorageHandler *mStorageHandler;
};


#endif //GEO_NETWORK_CLIENT_GATEWAYNOTIFICATIONRECEIVERTRANSACTION_H
