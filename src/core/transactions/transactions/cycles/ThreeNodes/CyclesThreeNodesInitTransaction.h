#ifndef GEO_NETWORK_CLIENT_THREENODESINITTRANSACTION_H
#define GEO_NETWORK_CLIENT_THREENODESINITTRANSACTION_H

#include "../../base/BaseTransaction.h"
#include "../../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../../io/storage/RoutingTablesHandler.h"
#include "../../../../network/messages/cycles/ThreeNodes/CyclesThreeNodesBalancesRequestMessage.h"
#include "../../../../network/messages/cycles/ThreeNodes/CyclesThreeNodesBalancesResponseMessage.h"

#include <set>


class CyclesThreeNodesInitTransaction :
    public BaseTransaction {

public:
    CyclesThreeNodesInitTransaction(
        const NodeUUID &nodeUUID,
        const NodeUUID &contractorUUID,
        TrustLinesManager *manager,
        RoutingTablesHandler *routingTablesHandler,
        Logger *logger);

    TransactionResult::SharedConst run();

protected:
    enum Stages {
        CollectDataAndSendMessage = 1,
        ParseMessageAndCreateCycles
    };

    TransactionResult::SharedConst runCollectDataAndSendMessageStage();
    TransactionResult::SharedConst runParseMessageAndCreateCyclesStage();

protected:
    set<NodeUUID> getNeighborsWithContractor();

protected:
    NodeUUID mContractorUUID;
    TrustLinesManager *mTrustLinesManager;
    Logger *mLogger;
    RoutingTablesHandler *mRoutingTablesHandler;
};

#endif //GEO_NETWORK_CLIENT_THREENODESINITTRANSACTION_H
