#ifndef GEO_NETWORK_CLIENT_THREENODESINITTRANSACTION_H
#define GEO_NETWORK_CLIENT_THREENODESINITTRANSACTION_H

#include "../../base/BaseTransaction.h"
#include "../../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../../cycles/CyclesManager.h"
#include "../../../../cycles/RoutingTableManager.h"
#include "../../../../paths/lib/Path.h"
#include "../../../../network/messages/cycles/ThreeNodes/CyclesThreeNodesBalancesRequestMessage.h"
#include "../../../../network/messages/cycles/ThreeNodes/CyclesThreeNodesBalancesResponseMessage.h"

#include <set>


class CyclesThreeNodesInitTransaction :
    public BaseTransaction {

public:
    CyclesThreeNodesInitTransaction(
        const NodeUUID &nodeUUID,
        const NodeUUID &contractorUUID,
        const SerializedEquivalent equivalent,
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
    set<NodeUUID> getNeighborsWithContractor();

protected:
    NodeUUID mContractorUUID;
    TrustLinesManager *mTrustLinesManager;
    CyclesManager *mCyclesManager;
    RoutingTableManager *mRougingTable;
};

#endif //GEO_NETWORK_CLIENT_THREENODESINITTRANSACTION_H
