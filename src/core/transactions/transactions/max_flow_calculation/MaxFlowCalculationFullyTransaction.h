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
        ContractorsManager *contractorsManager,
        TrustLinesManager *trustLinesManager,
        TopologyTrustLinesManager *topologyTrustLineManager,
        TopologyCacheManager *topologyCacheManager,
        MaxFlowCacheManager *maxFlowCacheManager,
        bool iAmGateway,
        Logger &logger);

protected:
    const string logHeader() const;

private:
    TransactionResult::SharedConst sendRequestForCollectingTopology();

    TransactionResult::SharedConst processCollectingTopology();

    TransactionResult::SharedConst applyCustomLogic();

    TrustLineAmount calculateMaxFlow(
        const NodeUUID &contractorUUID);

    TrustLineAmount calculateMaxFlowNew(
        ContractorID contractorID);

    void calculateMaxFlowOnOneLevel();

    void calculateMaxFlowOnOneLevelNew();

    TrustLineAmount calculateOneNode(
        const NodeUUID& nodeUUID,
        const TrustLineAmount& currentFlow,
        byte level);

    TrustLineAmount calculateOneNodeNew(
        ContractorID nodeUUID,
        const TrustLineAmount& currentFlow,
        byte level);

    TransactionResult::SharedConst resultOk();

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
    vector<ContractorID> mForbiddenNodeIDs;
    byte mCurrentPathLength;
    TrustLineAmount mCurrentMaxFlow;
    NodeUUID mCurrentContractor;
    ContractorID mCurrentContractorNew;
    size_t mCountProcessCollectingTopologyRun;
    TopologyTrustLinesManager::TrustLineWithPtrHashSet mFirstLevelTopology;
    TopologyTrustLinesManager::TrustLineWithPtrHashSetNew mFirstLevelTopologyNew;
    vector<pair<NodeUUID, TrustLineAmount>> mMaxFlows;
    vector<pair<ContractorID, BaseAddress::Shared>> mContractorIDs;
    vector<pair<ContractorID, TrustLineAmount>> mMaxFlowsNew;
    size_t mCurrentGlobalContractorIdx;
    bool mIamGateway;
};


#endif //GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONFULLYTRANSACTION_H
