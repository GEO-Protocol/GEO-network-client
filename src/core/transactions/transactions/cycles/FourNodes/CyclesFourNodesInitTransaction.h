#ifndef GEO_NETWORK_CLIENT_GETFOURNODESNEIGHBORBALANCESTRANSACTION_H
#define GEO_NETWORK_CLIENT_GETFOURNODESNEIGHBORBALANCESTRANSACTION_H

#include "../../base/BaseTransaction.h"
#include "../../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../../cycles/CyclesManager.h"
#include "../../../../io/storage/StorageHandler.h"
#include "../../../../cycles/RoutingTableManager.h"
#include "../../../../network/messages/cycles/FourNodes/CyclesFourNodesBalancesRequestMessage.h"
#include "../../../../network/messages/cycles/FourNodes/CyclesFourNodesBalancesResponseMessage.h"
#include "../../../../paths/lib/Path.h"

#include <set>

class CyclesFourNodesInitTransaction :
    public BaseTransaction {

public:
    CyclesFourNodesInitTransaction(
        const NodeUUID &nodeUUID,
        const NodeUUID &creditorContractorUUID,
        TrustLinesManager *manager,
        RoutingTableManager *roughtingTable,
        CyclesManager *cyclesManager,
        StorageHandler *storageHandler,
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
    vector<NodeUUID> calculateCommonNodes(
        const NodeUUID &firstNode,
        const NodeUUID &secondNode);

protected:
    NodeUUID mCreditorContractorUUID;
    TrustLinesManager *mTrustLinesManager;
    CyclesManager *mCyclesManager;
    StorageHandler *mStorageHandler;
    RoutingTableManager *mRoughtingTable;

    map<NodeUUID, NodeUUID> mWaitingResponses;
};

#endif //GEO_NETWORK_CLIENT_GETFOURNODESNEIGHBORBALANCESTRANSACTION_H
