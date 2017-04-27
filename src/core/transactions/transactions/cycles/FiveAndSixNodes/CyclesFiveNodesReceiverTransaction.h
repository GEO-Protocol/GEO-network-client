#ifndef GEO_NETWORK_CLIENT_CYCLESFIVESIXNODESRESPOSETRANSACTION_H
#define GEO_NETWORK_CLIENT_CYCLESFIVESIXNODESRESPOSETRANSACTION_H

#include "../../base/BaseTransaction.h"
#include "../../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../../network/messages/cycles/SixAndFiveNodes/CyclesFiveNodesInBetweenMessage.hpp"
#include <set>

class CyclesFiveNodesReceiverTransaction : public BaseTransaction {
public:
    CyclesFiveNodesReceiverTransaction(
        const NodeUUID &nodeUUID,
        CyclesFiveNodesInBetweenMessage::Shared message,
        TrustLinesManager *manager,
        Logger *logger);

    TransactionResult::SharedConst run();

protected:
//    Nodes Balances that are mutual between core node and contract node
    CyclesFiveNodesInBetweenMessage::Shared mInBetweenNodeTopologyMessage;
    TrustLinesManager *mTrustLinesManager;
};
#endif //GEO_NETWORK_CLIENT_CYCLESFIVESIXNODESRESPOSETRANSACTION_H
