#ifndef GEO_NETWORK_CLIENT_CYCLESSIXNODESRESPONSETRANSACTION_H
#define GEO_NETWORK_CLIENT_CYCLESSIXNODESRESPONSETRANSACTION_H

#include "../../base/BaseTransaction.h"
#include "../../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../../io/storage/StorageHandler.h"
#include "../../../../network/messages/cycles/SixAndFiveNodes/CyclesSixNodesInBetweenMessage.hpp"
#include "../../../../network/messages/cycles/SixAndFiveNodes/CyclesSixNodesBoundaryMessage.hpp"
#include <set>

class CyclesSixNodesReceiverTransaction :
    public BaseTransaction {
public:
    CyclesSixNodesReceiverTransaction(
        const NodeUUID &nodeUUID,
        CyclesSixNodesInBetweenMessage::Shared message,
        TrustLinesManager *manager,
        Logger &logger);

    TransactionResult::SharedConst run();

protected:
    const string logHeader() const;

protected:
//    Nodes Balances that are mutual between core node and contract node
    CyclesSixNodesInBetweenMessage::Shared mInBetweenNodeTopologyMessage;
    TrustLinesManager *mTrustLinesManager;
};
#endif //GEO_NETWORK_CLIENT_CYCLESSIXNODESRESPONSETRANSACTION_H
