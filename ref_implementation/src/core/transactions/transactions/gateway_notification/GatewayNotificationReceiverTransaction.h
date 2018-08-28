/**
 * This file is part of GEO Protocol.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Protocol/GEO-network-client/blob/master/LICENSE.md
 *
 * No part of GEO Protocol, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

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
