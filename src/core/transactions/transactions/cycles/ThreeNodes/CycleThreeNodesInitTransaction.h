#ifndef GEO_NETWORK_CLIENT_THREENODESINITTRANSACTION_H
#define GEO_NETWORK_CLIENT_THREENODESINITTRANSACTION_H

#include "../../base/UniqueTransaction.h"
#include "../../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../../io/storage/StorageHandler.h"
#include "../../../../network/messages/cycles/ThreeNodes/ThreeNodesBalancesRequestMessage.h"
#include "../../../../network/messages/cycles/ThreeNodes/ThreeNodesBalancesResponseMessage.h"
#include <set>

class CycleThreeNodesInitTransaction : public UniqueTransaction {
public:
    CycleThreeNodesInitTransaction(
            const TransactionType type,
            const NodeUUID &nodeUUID,
            const NodeUUID &contractorUUID,
            TransactionsScheduler *scheduler,
            TrustLinesManager *manager,
            StorageHandler *handler,
            Logger *logger);

    CycleThreeNodesInitTransaction(TransactionsScheduler *scheduler);

    TransactionResult::SharedConst run();

    pair<BytesShared, size_t> serializeToBytes() const {};

protected:
    enum Stages {
        CollectDataAndSendMessage = 1,
        ParseMessageAndCreateCycles
    };

    TransactionResult::SharedConst runCollectDataAndSendMessageStage();
    TransactionResult::SharedConst runParseMessageAndCreateCyclesStage();
    set<NodeUUID> getNeighborsWithContractor();

protected:
    const uint16_t kResponseCodeSuccess = 200;
    const uint8_t kMaxRequestsCount = 5;
    const uint16_t kStandardConnectionTimeout = 2000;

    uint16_t mRequestCounter = 0;
    uint32_t mConnectionTimeout = kStandardConnectionTimeout;
//    Nodes Balances that are mutual between core node and contract node
    NodeUUID mContractorUUID;
    TrustLinesManager *mTrustLinesManager;
    Logger *mlogger;
    StorageHandler *mStorageHandler;
};

#endif //GEO_NETWORK_CLIENT_THREENODESINITTRANSACTION_H
