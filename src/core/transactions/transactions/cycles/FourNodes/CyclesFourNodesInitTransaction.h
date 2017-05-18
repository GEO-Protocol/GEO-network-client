#ifndef GEO_NETWORK_CLIENT_GETFOURNODESNEIGHBORBALANCESTRANSACTION_H
#define GEO_NETWORK_CLIENT_GETFOURNODESNEIGHBORBALANCESTRANSACTION_H

#include "../../base/BaseTransaction.h"
#include "../../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../../max_flow_calculation/cashe/MaxFlowCalculationCacheManager.h"
#include "../../../../io/storage/RoutingTablesHandler.h"
#include "../../../../network/messages/cycles/FourNodes/CyclesFourNodesBalancesRequestMessage.h"
#include "../../../../network/messages/cycles/FourNodes/CyclesFourNodesBalancesResponseMessage.h"
#include "../../../../paths/lib/Path.h"
#include "../../regular/payments/CycleCloserInitiatorTransaction.h"

#include <set>

class CyclesFourNodesInitTransaction :
    public BaseTransaction {

public:
    CyclesFourNodesInitTransaction(
        const NodeUUID &nodeUUID,
        const NodeUUID &debtorContractorUUID,
        const NodeUUID &creditorContractorUUID,
        TrustLinesManager *manager,
        StorageHandler *storageHandler,
        MaxFlowCalculationCacheManager *maxFlowCalculationCacheManager,
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
    const string logHeader() const;
    set<NodeUUID> commonNeighborsForDebtorAndCreditorNodes();

protected:
    NodeUUID mDebtorContractorUUID;
    NodeUUID mCreditorContractorUUID;
    TrustLinesManager *mTrustLinesManager;
    StorageHandler *mStorageHandler;
    MaxFlowCalculationCacheManager *mMaxFlowCalculationCacheManager;
};

#endif //GEO_NETWORK_CLIENT_GETFOURNODESNEIGHBORBALANCESTRANSACTION_H
