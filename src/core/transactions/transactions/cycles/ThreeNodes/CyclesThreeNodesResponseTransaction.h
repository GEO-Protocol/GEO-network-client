#ifndef GEO_NETWORK_CLIENT_THREENODESRESPONSETRANSACTION_H
#define GEO_NETWORK_CLIENT_THREENODESRESPONSETRANSACTION_H

#include "../../base/BaseTransaction.h"
#include "../../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../../io/storage/StorageHandler.h"
#include "../../../../network/messages/cycles/ThreeNodes/ThreeNodesBalancesRequestMessage.h"
#include "../../../../network/messages/cycles/ThreeNodes/ThreeNodesBalancesResponseMessage.h"

#include <set>


class CyclesThreeNodesResponseTransaction :
    public BaseTransaction {

public:
    CyclesThreeNodesResponseTransaction(
        const NodeUUID &nodeUUID,
        ThreeNodesBalancesRequestMessage::Shared message,
        TrustLinesManager *manager,
        Logger *logger);

    TransactionResult::SharedConst run();

protected:
    ThreeNodesBalancesRequestMessage::Shared mRequestMessage;
    TrustLinesManager *mTrustLinesManager;
    Logger *mLogger;
};
#endif //GEO_NETWORK_CLIENT_THREENODESRESPONSETRANSACTION_H
