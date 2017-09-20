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
