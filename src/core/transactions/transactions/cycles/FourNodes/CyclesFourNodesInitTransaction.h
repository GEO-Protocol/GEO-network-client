#ifndef GEO_NETWORK_CLIENT_GETFOURNODESNEIGHBORBALANCESTRANSACTION_H
#define GEO_NETWORK_CLIENT_GETFOURNODESNEIGHBORBALANCESTRANSACTION_H

#include "../../base/BaseTransaction.h"
#include "../../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../../io/storage/RoutingTablesHandler.h"
#include "../../../../network/messages/cycles/FourNodes/FourNodesBalancesRequestMessage.h"
#include "../../../../network/messages/cycles/FourNodes/FourNodesBalancesResponseMessage.h"
#include <set>

class CyclesFourNodesInitTransaction :
    public BaseTransaction {

public:
    CyclesFourNodesInitTransaction(
            const NodeUUID &nodeUUID,
            const NodeUUID &debtorContractorUUID,
            const NodeUUID &creditorContractorUUID,
            TrustLinesManager *manager,
            RoutingTablesHandler *routingTablesHandler,
            Logger *logger);

    TransactionResult::SharedConst run();

    enum Stages {
        CollectDataAndSendMessage = 1,
        ParseMessageAndCreateCycles
    };

    TransactionResult::SharedConst runCollectDataAndSendMessageStage();
    TransactionResult::SharedConst runParseMessageAndCreateCyclesStage();

protected:
    set<NodeUUID> getCommonNeighborsForDebtorAndCreditorNodes();

private:
//    Nodes Balances that are mutual between core node and contract node
    NodeUUID mDebtorContractorUUID;
    NodeUUID mCreditorContractorUUID;
    TrustLinesManager *mTrustLinesManager;
    Logger *mLogger;
    RoutingTablesHandler *mRoutingTablesHandler;
};

#endif //GEO_NETWORK_CLIENT_GETFOURNODESNEIGHBORBALANCESTRANSACTION_H
