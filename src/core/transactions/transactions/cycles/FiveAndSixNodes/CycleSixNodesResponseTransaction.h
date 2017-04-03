#ifndef GEO_NETWORK_CLIENT_CYCLESSIXNODESRESPONSETRANSACTION_H
#define GEO_NETWORK_CLIENT_CYCLESSIXNODESRESPONSETRANSACTION_H
#include "../../base/UniqueTransaction.h"
#include "../../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../../io/storage/StorageHandler.h"
#include "../../../../network/messages/cycles/SixAndFiveNodes/CycleSixNodesInBetweenMessage.hpp"
#include <set>

class CycleSixNodesResponseTransaction : public UniqueTransaction {
public:
    CycleSixNodesResponseTransaction(
        const TransactionType type,
        const NodeUUID &nodeUUID,
        const NodeUUID &contractorUUID,
        CycleSixNodesInBetweenMessage::Shared message,
        TransactionsScheduler *scheduler,
        TrustLinesManager *manager,
        Logger *logger);

    CycleSixNodesResponseTransaction(TransactionsScheduler *scheduler);

    TransactionResult::SharedConst run();

    pair<BytesShared, size_t> serializeToBytes() const {};

protected:
//    Nodes Balances that are mutual between core node and contract node
    CycleSixNodesInBetweenMessage::Shared mInBetweenNodeTopologyMessage;
    NodeUUID mContractorUUID;
    TrustLinesManager *mTrustLinesManager;
    Logger *mlogger;
    StorageHandler *mStorageHandler;

};
#endif //GEO_NETWORK_CLIENT_CYCLESSIXNODESRESPONSETRANSACTION_H
