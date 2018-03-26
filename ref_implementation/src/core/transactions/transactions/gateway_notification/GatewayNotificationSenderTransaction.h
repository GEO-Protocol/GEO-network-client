/**
 * This file is part of GEO Project.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Project/GEO-Project/blob/master/LICENSE.md
 *
 * No part of GEO Project, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

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
