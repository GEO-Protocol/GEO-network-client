#ifndef GEO_NETWORK_CLIENT_GETFOURNODESNEIGHBORBALANCESTRANSACTION_H
#define GEO_NETWORK_CLIENT_GETFOURNODESNEIGHBORBALANCESTRANSACTION_H

#include "../../../base/UniqueTransaction.h"
#include "../../../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../../../network/messages/cycles/FourNodes/FourNodesBalancesRequestMessage.h"
#include "../../../../../network/messages/cycles/FourNodes/FourNodesBalancesResponseMessage.h"
#include <set>

class GetFourNodesNeighborBalancesTransaction : public UniqueTransaction {

public:
    GetFourNodesNeighborBalancesTransaction(
            const TransactionType type,
            const NodeUUID &nodeUUID,
            const NodeUUID &debtorContractorUUID,
            const NodeUUID &creditorContractorUUID,
            TransactionsScheduler *scheduler,
            TrustLinesManager *manager,
            Logger *logger);

    GetFourNodesNeighborBalancesTransaction(
            const TransactionType type,
            const NodeUUID &nodeUUID,
            FourNodesBalancesRequestMessage::Shared message,
            TransactionsScheduler *scheduler,
            TrustLinesManager *manager,
            Logger *logger);

    GetFourNodesNeighborBalancesTransaction(TransactionsScheduler *scheduler);

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
    FourNodesBalancesRequestMessage::Shared mRequestMessage = nullptr;
    NodeUUID mDebtorContractorUUID;
    NodeUUID mCreditorContractorUUID;
    TrustLinesManager *mTrustLinesManager;
    Logger *mlogger;
};

#endif //GEO_NETWORK_CLIENT_GETFOURNODESNEIGHBORBALANCESTRANSACTION_H
