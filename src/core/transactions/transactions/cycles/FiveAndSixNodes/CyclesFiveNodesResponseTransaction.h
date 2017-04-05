#ifndef GEO_NETWORK_CLIENT_CYCLESFIVESIXNODESRESPOSETRANSACTION_H
#define GEO_NETWORK_CLIENT_CYCLESFIVESIXNODESRESPOSETRANSACTION_H

#include "../../base/BaseTransaction.h"
#include "../../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../../network/messages/cycles/SixAndFiveNodes/CycleFiveNodesInBetweenMessage.hpp"
#include <set>

class CyclesFiveNodesResponseTransaction : public BaseTransaction {
public:
    CyclesFiveNodesResponseTransaction(
        const NodeUUID &nodeUUID,
        CycleFiveNodesInBetweenMessage::Shared message,
        TrustLinesManager *manager,
        Logger *logger);

    TransactionResult::SharedConst run();

protected:
//    Nodes Balances that are mutual between core node and contract node
    CycleFiveNodesInBetweenMessage::Shared mInBetweenNodeTopologyMessage;
    TrustLinesManager *mTrustLinesManager;
    Logger *mLogger;
};
#endif //GEO_NETWORK_CLIENT_CYCLESFIVESIXNODESRESPOSETRANSACTION_H
