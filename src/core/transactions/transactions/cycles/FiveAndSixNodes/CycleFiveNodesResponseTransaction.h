#ifndef GEO_NETWORK_CLIENT_CYCLESFIVESIXNODESRESPOSETRANSACTION_H
#define GEO_NETWORK_CLIENT_CYCLESFIVESIXNODESRESPOSETRANSACTION_H
#include "../../base/UniqueTransaction.h"
#include "../../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../../io/storage/StorageHandler.h"
#include "../../../../network/messages/cycles/SixAndFiveNodes/CycleFiveNodesInBetweenMessage.hpp"
#include <set>

class CycleFiveNodesResponseTransaction : public UniqueTransaction {
public:
    CycleFiveNodesResponseTransaction(
        const TransactionType type,
        const NodeUUID &nodeUUID,
        const NodeUUID &contractorUUID,
        CycleFiveNodesInBetweenMessage::Shared message,
        TransactionsScheduler *scheduler,
        TrustLinesManager *manager,
        Logger *logger);

    CycleFiveNodesResponseTransaction(TransactionsScheduler *scheduler);

    TransactionResult::SharedConst run();

    pair<BytesShared, size_t> serializeToBytes() const {};

protected:
//    Nodes Balances that are mutual between core node and contract node
    CycleFiveNodesInBetweenMessage::Shared mInBetweenNodeTopologyMessage;
    NodeUUID mContractorUUID;
    TrustLinesManager *mTrustLinesManager;
    Logger *mlogger;
    StorageHandler *mStorageHandler;

};
#endif //GEO_NETWORK_CLIENT_CYCLESFIVESIXNODESRESPOSETRANSACTION_H
