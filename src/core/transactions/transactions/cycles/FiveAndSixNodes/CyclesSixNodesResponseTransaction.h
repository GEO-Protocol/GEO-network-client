#ifndef GEO_NETWORK_CLIENT_CYCLESSIXNODESRESPONSETRANSACTION_H
#define GEO_NETWORK_CLIENT_CYCLESSIXNODESRESPONSETRANSACTION_H

#include "../../base/BaseTransaction.h"
#include "../../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../../io/storage/StorageHandler.h"
#include "../../../../network/messages/cycles/SixAndFiveNodes/CyclesSixNodesInBetweenMessage.hpp"
#include <set>

class CyclesSixNodesResponseTransaction :
    public BaseTransaction {
public:
    CyclesSixNodesResponseTransaction(
        const NodeUUID &nodeUUID,
        CyclesSixNodesInBetweenMessage::Shared message,
        TrustLinesManager *manager,
        Logger *logger);

    TransactionResult::SharedConst run();

protected:
//    Nodes Balances that are mutual between core node and contract node
    CyclesSixNodesInBetweenMessage::Shared mInBetweenNodeTopologyMessage;
    TrustLinesManager *mTrustLinesManager;
    Logger *mLogger;
};
#endif //GEO_NETWORK_CLIENT_CYCLESSIXNODESRESPONSETRANSACTION_H
