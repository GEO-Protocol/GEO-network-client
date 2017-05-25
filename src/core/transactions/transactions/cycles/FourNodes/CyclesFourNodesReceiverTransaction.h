#ifndef GEO_NETWORK_CLIENT_CYCLEFOURNODESRESPONSETRANSACTION_H
#define GEO_NETWORK_CLIENT_CYCLEFOURNODESRESPONSETRANSACTION_H

#include "../../base/BaseTransaction.h"
#include "../../../../trust_lines/manager/TrustLinesManager.h"

#include "../../../../network/messages/cycles/FourNodes/CyclesFourNodesBalancesRequestMessage.h"
#include "../../../../network/messages/cycles/FourNodes/CyclesFourNodesBalancesResponseMessage.h"

#include <set>

class CyclesFourNodesReceiverTransaction:
    public BaseTransaction {

public:
    CyclesFourNodesReceiverTransaction(
        const NodeUUID &nodeUUID,
        CyclesFourNodesBalancesRequestMessage::Shared message,
        TrustLinesManager *manager,
        Logger &logger);

    TransactionResult::SharedConst run();

protected:
    const string logHeader() const;

protected:
    CyclesFourNodesBalancesRequestMessage::Shared mRequestMessage;
    TrustLinesManager *mTrustLinesManager;
};
#endif //GEO_NETWORK_CLIENT_CYCLEFOURNODESRESPONSETRANSACTION_H
