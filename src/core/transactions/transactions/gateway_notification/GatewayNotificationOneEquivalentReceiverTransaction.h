#ifndef GEO_NETWORK_CLIENT_GATEWAYNOTIFICATIONONEEQUIVALENTRECEIVERTRANSACTION_H
#define GEO_NETWORK_CLIENT_GATEWAYNOTIFICATIONONEEQUIVALENTRECEIVERTRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../io/storage/StorageHandler.h"
#include "../../../network/messages/gateway_notification/GatewayNotificationOneEquivalentMessage.h"
#include "../../../network/messages/base/transaction/ConfirmationMessage.h"

class GatewayNotificationOneEquivalentReceiverTransaction :
    public BaseTransaction {

public:
    typedef shared_ptr<GatewayNotificationOneEquivalentReceiverTransaction> Shared;

public:
    GatewayNotificationOneEquivalentReceiverTransaction(
        const NodeUUID &nodeUUID,
        GatewayNotificationOneEquivalentMessage::Shared message,
        TrustLinesManager *manager,
        StorageHandler *storageHandler,
        Logger &logger);

    TransactionResult::SharedConst run();

protected:
    const string logHeader() const;

private:
    GatewayNotificationOneEquivalentMessage::Shared mMessage;
    TrustLinesManager *mTrustLineManager;
    StorageHandler *mStorageHandler;
};


#endif //GEO_NETWORK_CLIENT_GATEWAYNOTIFICATIONONEEQUIVALENTRECEIVERTRANSACTION_H
