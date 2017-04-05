#ifndef GEO_NETWORK_CLIENT_CYCLEFOURNODESRESPONSETRANSACTION_H
#define GEO_NETWORK_CLIENT_CYCLEFOURNODESRESPONSETRANSACTION_H
#include "../../base/UniqueTransaction.h"
#include "../../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../../network/messages/cycles/FourNodes/FourNodesBalancesRequestMessage.h"
#include "../../../../network/messages/cycles/FourNodes/FourNodesBalancesResponseMessage.h"
#include <set>

class CycleFourNodesResponseTransaction : public UniqueTransaction {
public:
    CycleFourNodesResponseTransaction(
        const NodeUUID &nodeUUID,
        FourNodesBalancesRequestMessage::Shared message,
        TransactionsScheduler *scheduler,
        TrustLinesManager *manager,
        Logger *logger);

    CycleFourNodesResponseTransaction(TransactionsScheduler *scheduler);

    TransactionResult::SharedConst run();

    pair<BytesShared, size_t> serializeToBytes() const {};


protected:
//    Nodes Balances that are mutual between core node and contract node
    FourNodesBalancesRequestMessage::Shared mRequestMessage;
    TrustLinesManager *mTrustLinesManager;
    Logger *mLogger;
};
#endif //GEO_NETWORK_CLIENT_CYCLEFOURNODESRESPONSETRANSACTION_H
