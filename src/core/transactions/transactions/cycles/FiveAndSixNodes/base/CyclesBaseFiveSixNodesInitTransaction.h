#ifndef GEO_NETWORK_CLIENT_CYCLESFIVESIXNODESINITTRANSACTION_H
#define GEO_NETWORK_CLIENT_CYCLESFIVESIXNODESINITTRANSACTION_H

#include "../../../base/BaseTransaction.h"
#include "../../../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../../../max_flow_calculation/cashe/MaxFlowCalculationCacheManager.h"

class CyclesBaseFiveSixNodesInitTransaction :
    public BaseTransaction {

public:
    typedef shared_ptr<CyclesBaseFiveSixNodesInitTransaction> Shared;
    typedef multimap<NodeUUID, const shared_ptr<vector<NodeUUID>>> CycleMap;

public:
    CyclesBaseFiveSixNodesInitTransaction(
        const TransactionType type,
        const NodeUUID &nodeUUID,
        TrustLinesManager *manager,
        StorageHandler *storageHandler,
        MaxFlowCalculationCacheManager *maxFlowCalculationCacheManager,
        Logger *logger);

    TransactionResult::SharedConst run();

protected:
    enum Stages {
        CollectDataAndSendMessage = 1,
        ParseMessageAndCreateCycles
    };

    virtual TransactionResult::SharedConst runCollectDataAndSendMessagesStage() = 0;
    virtual TransactionResult::SharedConst runParseMessageAndCreateCyclesStage() = 0;
    virtual const string logHeader() const = 0;

protected:
    TrustLinesManager *mTrustLinesManager;
    StorageHandler *mStorageHandler;
    MaxFlowCalculationCacheManager *mMaxFlowCalculationCacheManager;

};

#endif //GEO_NETWORK_CLIENT_CYCLESFIVESIXNODESINITTRANSACTION_H
