#ifndef GEO_NETWORK_CLIENT_THREENODESRESPONSETRANSACTION_H
#define GEO_NETWORK_CLIENT_THREENODESRESPONSETRANSACTION_H

#include "../../base/UniqueTransaction.h"
#include "../../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../../io/storage/StorageHandler.h"
#include "../../../../network/messages/cycles/ThreeNodes/ThreeNodesBalancesRequestMessage.h"
#include "../../../../network/messages/cycles/ThreeNodes/ThreeNodesBalancesResponseMessage.h"
#include <set>

class CycleThreeNodesResponseTransaction : public UniqueTransaction {
public:
    CycleThreeNodesResponseTransaction(
            const NodeUUID &nodeUUID,
            ThreeNodesBalancesRequestMessage::Shared message,
            TransactionsScheduler *scheduler,
            TrustLinesManager *manager,
            Logger *logger);

    CycleThreeNodesResponseTransaction(TransactionsScheduler *scheduler);

    TransactionResult::SharedConst run();

    pair<BytesShared, size_t> serializeToBytes() const {};


protected:
//    Nodes Balances that are mutual between core node and contract node
    ThreeNodesBalancesRequestMessage::Shared mRequestMessage;
    NodeUUID mContractorUUID;
    TrustLinesManager *mTrustLinesManager;
    Logger *mLogger;
    StorageHandler *mStorageHandler;

};
#endif //GEO_NETWORK_CLIENT_THREENODESRESPONSETRANSACTION_H
