#ifndef GEO_NETWORK_CLIENT_INITIATEMAXFLOWCALCULATIONTRANSACTION_H
#define GEO_NETWORK_CLIENT_INITIATEMAXFLOWCALCULATIONTRANSACTION_H

#include "../base/BaseCollectTopologyTransaction.h"
#include "../../../interface/commands_interface/commands/max_flow_calculation/InitiateMaxFlowCalculationCommand.h"

#include "../../../network/messages/max_flow_calculation/InitiateMaxFlowCalculationMessage.h"
#include "../../../network/messages/max_flow_calculation/MaxFlowCalculationSourceFstLevelMessage.h"
#include "../../../network/messages/max_flow_calculation/ResultMaxFlowCalculationMessage.h"

#include "CollectTopologyTransaction.h"

#include <set>

class InitiateMaxFlowCalculationTransaction : public BaseCollectTopologyTransaction {

public:
    typedef shared_ptr<InitiateMaxFlowCalculationTransaction> Shared;

public:
    InitiateMaxFlowCalculationTransaction(
        NodeUUID &nodeUUID,
        InitiateMaxFlowCalculationCommand::Shared command,
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

    TrustLineAmount calculateMaxFlowUpdated(
        const NodeUUID &contractorUUID);

    void calculateMaxFlowOnOneLevel();

    void calculateMaxFlowOnOneLevelUpdated();

    TrustLineAmount calculateOneNode(
        const NodeUUID& nodeUUID,
        const TrustLineAmount& currentFlow,
        byte level);

    TransactionResult::SharedConst resultFinalOk();

    TransactionResult::SharedConst resultIntermediateOk();

    TransactionResult::SharedConst resultProtocolError();

private:
    static const byte kShortMaxPathLength = 5;
    static const byte kLongMaxPathLength = 6;
    static const uint32_t kWaitMillisecondsForCalculatingMaxFlow = 1000;
    static const uint32_t kWaitMillisecondsForCalculatingMaxFlowAgain = 500;
    static const uint32_t kMaxWaitMillisecondsForCalculatingMaxFlow = 10000;
    static const uint16_t kCountRunningProcessCollectingTopologyStage =
            (kMaxWaitMillisecondsForCalculatingMaxFlow - kWaitMillisecondsForCalculatingMaxFlow * 2) /
            kWaitMillisecondsForCalculatingMaxFlowAgain;

private:
    InitiateMaxFlowCalculationCommand::Shared mCommand;
    vector<NodeUUID> mForbiddenNodeUUIDs;
    byte mCurrentPathLength;
    TrustLineAmount mCurrentMaxFlow;
    NodeUUID mCurrentContractor;
    size_t mCountProcessCollectingTopologyRun;
    TopologyTrustLinesManager::TrustLineWithPtrHashSet mFirstLevelTopology;
    map<NodeUUID, TrustLineAmount> mMaxFlows;
    uint16_t mResultStep;
    bool mShortMaxFlowsCalculated;
    bool mGatewayResponseProcessed;
    size_t mCurrentGlobalContractorIdx;
    bool mFinalTopologyCollected;
    byte mMaxPathLength;
    bool mIamGateway;
};


#endif //GEO_NETWORK_CLIENT_INITIATEMAXFLOWCALCULATIONTRANSACTION_H
