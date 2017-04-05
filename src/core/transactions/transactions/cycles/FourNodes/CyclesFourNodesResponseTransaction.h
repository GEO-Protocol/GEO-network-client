#ifndef GEO_NETWORK_CLIENT_CYCLEFOURNODESRESPONSETRANSACTION_H
#define GEO_NETWORK_CLIENT_CYCLEFOURNODESRESPONSETRANSACTION_H

#include "../../base/BaseTransaction.h"
#include "../../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../../network/messages/cycles/FourNodes/FourNodesBalancesRequestMessage.h"
#include "../../../../network/messages/cycles/FourNodes/FourNodesBalancesResponseMessage.h"

#include <set>

class CyclesFourNodesResponseTransaction :
    public BaseTransaction{
public:
    CyclesFourNodesResponseTransaction(
        const NodeUUID &nodeUUID,
        FourNodesBalancesRequestMessage::Shared message,
        TrustLinesManager *manager,
        Logger *logger);

    TransactionResult::SharedConst run();

protected:
//    Nodes Balances that are mutual between core node and contract node
    FourNodesBalancesRequestMessage::Shared mRequestMessage;
    TrustLinesManager *mTrustLinesManager;
    Logger *mLogger;
};
#endif //GEO_NETWORK_CLIENT_CYCLEFOURNODESRESPONSETRANSACTION_H
