#ifndef GEO_NETWORK_CLIENT_CYCLESSIXNODESRESPONSETRANSACTION_H
#define GEO_NETWORK_CLIENT_CYCLESSIXNODESRESPONSETRANSACTION_H

#include "../../base/BaseTransaction.h"
#include "../../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../../io/storage/StorageHandler.h"
#include "../../../../network/messages/cycles/SixAndFiveNodes/CycleSixNodesInBetweenMessage.hpp"
#include <set>

class CyclesSixNodesResponseTransaction :
    public BaseTransaction {
public:
    CyclesSixNodesResponseTransaction(
        const NodeUUID &nodeUUID,
        CycleSixNodesInBetweenMessage::Shared message,
        TrustLinesManager *manager,
        Logger *logger);

    TransactionResult::SharedConst run();

protected:
//    Nodes Balances that are mutual between core node and contract node
    CycleSixNodesInBetweenMessage::Shared mInBetweenNodeTopologyMessage;
    TrustLinesManager *mTrustLinesManager;
    Logger *mLogger;
};
#endif //GEO_NETWORK_CLIENT_CYCLESSIXNODESRESPONSETRANSACTION_H
