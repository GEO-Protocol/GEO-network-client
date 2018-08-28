/**
 * This file is part of GEO Protocol.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Protocol/GEO-network-client/blob/master/LICENSE.md
 *
 * No part of GEO Protocol, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#ifndef GEO_NETWORK_CLIENT_ROUTINGTALESINITTRANSACTION_H
#define GEO_NETWORK_CLIENT_ROUTINGTALESINITTRANSACTION_H

#include "../base/BaseTransaction.h"

#include "../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../cycles/RoutingTableManager.h"

class RoutingTableInitTransaction:
    public BaseTransaction {

public:
    RoutingTableInitTransaction(
        const NodeUUID &nodeUUID,
        TrustLinesManager *trustlineManager,
        RoutingTableManager *routingTableManager,
        Logger &logger);

    TransactionResult::SharedConst run();

protected:
    TransactionResult::SharedConst runCollectDataStage();
    TransactionResult::SharedConst runUpdateRoutingTableStage();

protected:
    enum Stages {
        CollectDataStage = 1,
        UpdateRoutingTableStage
    };

    const string logHeader() const;

protected:
    NodeUUID mNodeUUID;
    TrustLinesManager *mTrustlineManager;
    RoutingTableManager *mRoutingTableManager;
    Logger &mLog;
};
#endif //GEO_NETWORK_CLIENT_ROUTINGTALESINITTRANSACTION_H
