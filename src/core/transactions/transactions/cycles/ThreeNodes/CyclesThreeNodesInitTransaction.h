#ifndef GEO_NETWORK_CLIENT_THREENODESINITTRANSACTION_H
#define GEO_NETWORK_CLIENT_THREENODESINITTRANSACTION_H

#include "../../base/BaseTransaction.h"
#include "../../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../../max_flow_calculation/cashe/MaxFlowCalculationCacheManager.h"
#include "../../../../io/storage/RoutingTablesHandler.h"
#include "../../../../paths/lib/Path.h"
#include "../../../../network/messages/cycles/ThreeNodes/CyclesThreeNodesBalancesRequestMessage.h"
#include "../../../../network/messages/cycles/ThreeNodes/CyclesThreeNodesBalancesResponseMessage.h"

#include "../../regular/payments/CycleCloserInitiatorTransaction.h"

#include <set>


class CyclesThreeNodesInitTransaction :
    public BaseTransaction {

public:
    CyclesThreeNodesInitTransaction(
        const NodeUUID &nodeUUID,
        const NodeUUID &contractorUUID,
        TrustLinesManager *manager,
        StorageHandler *storageHandler,
        MaxFlowCalculationCacheManager *maxFlowCalculationCacheManager,
        Logger &logger);

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
    set<NodeUUID> getNeighborsWithContractor();

protected:
    NodeUUID mContractorUUID;
    TrustLinesManager *mTrustLinesManager;
    StorageHandler *mStorageHandler;
    MaxFlowCalculationCacheManager *mMaxFlowCalculationCacheManager;
};

#endif //GEO_NETWORK_CLIENT_THREENODESINITTRANSACTION_H
