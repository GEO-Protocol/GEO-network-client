#ifndef GEO_NETWORK_CLIENT_CYCLEFOURNODESRESPONSETRANSACTION_H
#define GEO_NETWORK_CLIENT_CYCLEFOURNODESRESPONSETRANSACTION_H

#include "../../base/BaseTransaction.h"
#include "../../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../../network/messages/cycles/FourNodes/CyclesFourNodesBalancesRequestMessage.h"
#include "../../../../network/messages/cycles/FourNodes/CyclesFourNodesBalancesResponseMessage.h"

#include <set>

class CyclesFourNodesResponseTransaction:
    public BaseTransaction {

public:
    CyclesFourNodesResponseTransaction(
        const NodeUUID &nodeUUID,
        CyclesFourNodesBalancesRequestMessage::Shared message,
        TrustLinesManager *manager,
        Logger *logger);

    TransactionResult::SharedConst run();

protected:
    CyclesFourNodesBalancesRequestMessage::Shared mRequestMessage;
    TrustLinesManager *mTrustLinesManager;
    Logger *mLogger;
};
#endif //GEO_NETWORK_CLIENT_CYCLEFOURNODESRESPONSETRANSACTION_H
