#ifndef GEO_NETWORK_CLIENT_THREENODESRESPONSETRANSACTION_H
#define GEO_NETWORK_CLIENT_THREENODESRESPONSETRANSACTION_H

#include "../../base/BaseTransaction.h"
#include "../../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../../io/storage/StorageHandler.h"
#include "../../../../network/messages/cycles/ThreeNodes/CyclesThreeNodesBalancesRequestMessage.h"
#include "../../../../network/messages/cycles/ThreeNodes/CyclesThreeNodesBalancesResponseMessage.h"

#include <set>


class CyclesThreeNodesReceiverTransaction :
    public BaseTransaction {

public:
    CyclesThreeNodesReceiverTransaction(
        const NodeUUID &nodeUUID,
        CyclesThreeNodesBalancesRequestMessage::Shared message,
        TrustLinesManager *manager,
        Logger &logger);

    TransactionResult::SharedConst run();

protected:
    const string logHeader() const;

protected:
    CyclesThreeNodesBalancesRequestMessage::Shared mRequestMessage;
    TrustLinesManager *mTrustLinesManager;
};
#endif //GEO_NETWORK_CLIENT_THREENODESRESPONSETRANSACTION_H
