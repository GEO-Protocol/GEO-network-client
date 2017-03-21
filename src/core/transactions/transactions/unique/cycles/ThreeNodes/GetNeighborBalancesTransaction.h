#ifndef GEO_NETWORK_CLIENT_GETNEIGHBORTRANSACTION_H
#define GEO_NETWORK_CLIENT_GETNEIGHBORTRANSACTION_H

#include "../../../base/UniqueTransaction.h"
#include "../../../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../../../network/messages/cycles/ThreeNodes/ThreeNodesBalancesRequestMessage.h"
#include "../../../../../network/messages/cycles/ThreeNodes/ThreeNodesBalancesResponseMessage.h"

class GetNeighborBalancesTransaction : public UniqueTransaction {

public:
    GetNeighborBalancesTransaction(
            const TransactionType type,
            const NodeUUID &nodeUUID,
            const NodeUUID &contractorUUID,
            TransactionsScheduler *scheduler,
            TrustLinesManager *manager,
            Logger *logger);

    GetNeighborBalancesTransaction(
            const TransactionType type,
            const NodeUUID &nodeUUID,
            ThreeNodesBalancesRequestMessage::Shared message,
            TransactionsScheduler *scheduler,
            TrustLinesManager *manager,
            Logger *logger);

    GetNeighborBalancesTransaction(TransactionsScheduler *scheduler);

    TransactionResult::SharedConst run();

    TransactionResult::SharedConst waitingForNeighborBalances();
    pair<BytesShared, size_t> serializeToBytes() const {};

private:
    const uint16_t kResponseCodeSuccess = 200;
    const uint16_t kMaxRequestsCount = 5;
    const uint8_t kConnectionProgression = 2;
    const uint32_t kStandardConnectionTimeout = 20000;

    uint16_t mRequestCounter = 0;
    uint32_t mConnectionTimeout = kStandardConnectionTimeout;
//    Nodes Balances that are mutual between core node and contract node
    ThreeNodesBalancesRequestMessage::Shared mRequestMessage = nullptr;
    NodeUUID mContractorUUID;
    TrustLinesManager *mTrustLinesManager;
    Logger *mlogger;
};

#endif //GEO_NETWORK_CLIENT_GETNEIGHBORTRANSACTION_H
