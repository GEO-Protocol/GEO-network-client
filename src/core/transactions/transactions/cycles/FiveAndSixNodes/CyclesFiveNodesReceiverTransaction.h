#ifndef GEO_NETWORK_CLIENT_CYCLESFIVESIXNODESRESPOSETRANSACTION_H
#define GEO_NETWORK_CLIENT_CYCLESFIVESIXNODESRESPOSETRANSACTION_H

#include "../../base/BaseTransaction.h"
#include "../../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../../network/messages/cycles/SixAndFiveNodes/CyclesFiveNodesInBetweenMessage.hpp"
#include "../../../../network/messages/cycles/SixAndFiveNodes/CyclesFiveNodesBoundaryMessage.hpp"

#include <set>

class CyclesFiveNodesReceiverTransaction : public BaseTransaction {
public:
    CyclesFiveNodesReceiverTransaction(
        const NodeUUID &nodeUUID,
        CyclesFiveNodesInBetweenMessage::Shared message,
        TrustLinesManager *manager,
        Logger &logger);

    TransactionResult::SharedConst run();

protected:
    const string logHeader() const;

protected:
    CyclesFiveNodesInBetweenMessage::Shared mInBetweenNodeTopologyMessage;
    TrustLinesManager *mTrustLinesManager;
};
#endif //GEO_NETWORK_CLIENT_CYCLESFIVESIXNODESRESPOSETRANSACTION_H
