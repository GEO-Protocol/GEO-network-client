#ifndef GEO_NETWORK_CLIENT_GETFOURNODESNEIGHBORBALANCESTRANSACTION_H
#define GEO_NETWORK_CLIENT_GETFOURNODESNEIGHBORBALANCESTRANSACTION_H

#include "../../base/UniqueTransaction.h"
#include "../../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../../io/storage/StorageHandler.h"
#include "../../../../network/messages/cycles/FourNodes/FourNodesBalancesRequestMessage.h"
#include "../../../../network/messages/cycles/FourNodes/FourNodesBalancesResponseMessage.h"
#include <set>

class CyclesFourNodesInitTransaction : public UniqueTransaction {

public:
    CyclesFourNodesInitTransaction(
            const NodeUUID &nodeUUID,
            const NodeUUID &debtorContractorUUID,
            const NodeUUID &creditorContractorUUID,
            TransactionsScheduler *scheduler,
            TrustLinesManager *manager,
            StorageHandler *storageHandler,
            Logger *logger);

    CyclesFourNodesInitTransaction(TransactionsScheduler *scheduler);

    TransactionResult::SharedConst run();

    pair<BytesShared, size_t> serializeToBytes() const {};

    enum Stages {
        CollectDataAndSendMessage = 1,
        ParseMessageAndCreateCycles
    };

    TransactionResult::SharedConst runCollectDataAndSendMessageStage();
    TransactionResult::SharedConst runParseMessageAndCreateCyclesStage();
    set<NodeUUID> getCommonNeighborsForDebtorAndCreditorNodes();

private:
    const uint16_t kResponseCodeSuccess = 200;
    const uint16_t kMaxRequestsCount = 5;
    const uint8_t kConnectionProgression = 2;
    const uint16_t kStandardConnectionTimeout = 2000;

    uint16_t mRequestCounter = 0;
    uint32_t mConnectionTimeout = kStandardConnectionTimeout;
//    Nodes Balances that are mutual between core node and contract node
    NodeUUID mDebtorContractorUUID;
    NodeUUID mCreditorContractorUUID;
    TrustLinesManager *mTrustLinesManager;
    Logger *mlogger;
    StorageHandler *mStorageHandler;
};

#endif //GEO_NETWORK_CLIENT_GETFOURNODESNEIGHBORBALANCESTRANSACTION_H
