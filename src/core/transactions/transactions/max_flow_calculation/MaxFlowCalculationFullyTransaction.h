#ifndef GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONFULLYTRANSACTION_H
#define GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONFULLYTRANSACTION_H

#include "../base/BaseCollectTopologyTransaction.h"
#include "../../../interface/commands_interface/commands/max_flow_calculation/InitiateMaxFlowCalculationFullyCommand.h"

#include "CollectTopologyTransaction.h"

#include <set>

class MaxFlowCalculationFullyTransaction : public BaseCollectTopologyTransaction {

public:
    typedef shared_ptr<MaxFlowCalculationFullyTransaction> Shared;

public:
    MaxFlowCalculationFullyTransaction(
        const NodeUUID &nodeUUID,
        const InitiateMaxFlowCalculationFullyCommand::Shared command,
        TrustLinesManager *trustLinesManager,
        TopologyTrustLinesManager *topologyTrustLineManager,
        TopologyCacheManager *topologyCacheManager,
        MaxFlowCacheManager *maxFlowCacheManager,
        Logger &logger);

    InitiateMaxFlowCalculationFullyCommand::Shared command() const;

protected:
    const string logHeader() const;

private:
    TransactionResult::SharedConst sendRequestForCollectingTopology();

    TransactionResult::SharedConst processCollectingTopology();

    TransactionResult::SharedConst applyCustomLogic();

    TrustLineAmount calculateMaxFlow(
        const NodeUUID &contractorUUID);

    void calculateMaxFlowOnOneLevel();

    TrustLineAmount calculateOneNode(
        const NodeUUID& nodeUUID,
        const TrustLineAmount& currentFlow,
        byte level);

    TransactionResult::SharedConst resultOk(
        vector<pair<NodeUUID, TrustLineAmount>> &maxFlows);

    TransactionResult::SharedConst resultProtocolError();

private:
    static const byte kMaxPathLength = 6;
    static const uint32_t kWaitMillisecondsForCalculatingMaxFlow = 4000;
    static const uint32_t kWaitMillisecondsForCalculatingMaxFlowAgain = 500;
    static const uint32_t kMaxWaitMillisecondsForCalculatingMaxFlow = 15000;
    static const uint16_t kCountRunningProcessCollectingTopologyStage =
            (kMaxWaitMillisecondsForCalculatingMaxFlow - kWaitMillisecondsForCalculatingMaxFlow) /
            kWaitMillisecondsForCalculatingMaxFlowAgain;

private:
    InitiateMaxFlowCalculationFullyCommand::Shared mCommand;
    vector<NodeUUID> mForbiddenNodeUUIDs;
    byte mCurrentPathLength;
    TrustLineAmount mCurrentMaxFlow;
    NodeUUID mCurrentContractor;
    size_t mCountProcessCollectingTopologyRun;
    TopologyTrustLinesManager::TrustLineWithPtrHashSet mFirstLevelTopology;
    vector<pair<NodeUUID, TrustLineAmount>> maxFlows;
    size_t mCurrentGlobalContractorIdx;
};


#endif //GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONFULLYTRANSACTION_H
