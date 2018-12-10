#ifndef GEO_NETWORK_CLIENT_GETFOURNODESNEIGHBORBALANCESTRANSACTION_H
#define GEO_NETWORK_CLIENT_GETFOURNODESNEIGHBORBALANCESTRANSACTION_H

#include "../../base/BaseTransaction.h"
#include "../../../../contractors/ContractorsManager.h"
#include "../../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../../cycles/CyclesManager.h"
#include "../../../../cycles/RoutingTableManager.h"
#include "../../../../network/messages/cycles/FourNodes/CyclesFourNodesNegativeBalanceRequestMessage.h"
#include "../../../../network/messages/cycles/FourNodes/CyclesFourNodesPositiveBalanceRequestMessage.h"
#include "../../../../network/messages/cycles/FourNodes/CyclesFourNodesBalancesResponseMessage.h"
#include "../../../../paths/lib/Path.h"

#include <set>

class CyclesFourNodesInitTransaction :
    public BaseTransaction {

public:
    CyclesFourNodesInitTransaction(
        const NodeUUID &nodeUUID,
        ContractorID contractorID,
        const SerializedEquivalent equivalent,
        ContractorsManager *contractorsManager,
        TrustLinesManager *manager,
        RoutingTableManager *routingTable,
        CyclesManager *cyclesManager,
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

    vector<BaseAddress::Shared> getCommonNodes(
        BaseAddress::Shared creditorNeighborNode,
        vector<ContractorID> currentNodeSuitableNeighbors);

protected:
    ContractorID mContractorID;
    ContractorsManager *mContractorsManager;
    TrustLinesManager *mTrustLinesManager;
    CyclesManager *mCyclesManager;
    RoutingTableManager *mRoutingTable;
    bool mNegativeContractorBalance;

};

#endif //GEO_NETWORK_CLIENT_GETFOURNODESNEIGHBORBALANCESTRANSACTION_H
