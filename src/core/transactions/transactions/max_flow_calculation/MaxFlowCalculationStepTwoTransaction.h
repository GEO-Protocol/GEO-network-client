#ifndef GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONSTEPTWOTRANSACTION_H
#define GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONSTEPTWOTRANSACTION_H

#include "../base/BaseCollectTopologyTransaction.h"
#include "../../../interface/commands_interface/commands/max_flow_calculation/InitiateMaxFlowCalculationCommand.h"

#include "../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../max_flow_calculation/manager/MaxFlowCalculationTrustLineManager.h"
#include "../../../max_flow_calculation/cashe/MaxFlowCalculationCacheManager.h"
#include "../../../max_flow_calculation/cashe/MaxFlowCalculationNodeCacheManager.h"

#include <set>

class MaxFlowCalculationStepTwoTransaction : public BaseCollectTopologyTransaction {
public:
    typedef shared_ptr<MaxFlowCalculationStepTwoTransaction> Shared;

public:
    MaxFlowCalculationStepTwoTransaction(
        const NodeUUID &nodeUUID,
        const TransactionUUID &transactionUUID,
        const InitiateMaxFlowCalculationCommand::Shared command,
        TrustLinesManager *trustLinesManager,
        MaxFlowCalculationTrustLineManager *maxFlowCalculationTrustLineManager,
        MaxFlowCalculationCacheManager *maxFlowCalculationCacheManager,
        MaxFlowCalculationNodeCacheManager *maxFlowCalculationNodeCacheManager,
        Logger &logger);

    InitiateMaxFlowCalculationCommand::Shared command() const;

protected:
    const string logHeader() const;

private:
    TransactionResult::SharedConst sendRequestForCollectingTopology();

    TransactionResult::SharedConst processCollectingTopology();

    TrustLineAmount calculateMaxFlow(
        const NodeUUID &contractorUUID);

    void calculateMaxFlowOnOneLevel();

    TrustLineAmount calculateOneNode(
        const NodeUUID& nodeUUID,
        const TrustLineAmount& currentFlow,
        byte level);

    TransactionResult::SharedConst resultOk(
        vector<pair<NodeUUID, TrustLineAmount>> &maxFlows);

private:
    static const byte kMaxPathLength = 6;
    static const uint32_t kWaitMillisecondsForCalculatingMaxFlow = 1500;
    static const uint32_t kWaitMillisecondsForCalculatingMaxFlowAgain = 500;
    static const uint32_t kMaxWaitMillisecondsForCalculatingMaxFlow = 10000;
    static const uint16_t kCountRunningProcessCollectingTopologyStage =
            (kMaxWaitMillisecondsForCalculatingMaxFlow - kWaitMillisecondsForCalculatingMaxFlow) /
            kWaitMillisecondsForCalculatingMaxFlowAgain;

private:
    InitiateMaxFlowCalculationCommand::Shared mCommand;
    vector<NodeUUID> mForbiddenNodeUUIDs;
    byte mCurrentPathLength;
    TrustLineAmount mCurrentMaxFlow;
    NodeUUID mCurrentContractor;
    size_t mCountProcessCollectingTopologyRun;
    MaxFlowCalculationTrustLineManager::TrustLineWithPtrHashSet mFirstLevelTopology;
};


#endif //GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONSTEPTWOTRANSACTION_H
