#ifndef GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONTARGETSNDLEVELTRANSACTION_H
#define GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONTARGETSNDLEVELTRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../network/messages/max_flow_calculation/MaxFlowCalculationTargetSndLevelMessage.h"
#include "../../../network/messages/max_flow_calculation/ResultMaxFlowCalculationMessage.h"
#include "../../../network/messages/max_flow_calculation/ResultMaxFlowCalculationGatewayMessage.h"
#include "../../../topology/cashe/TopologyCacheManager.h"

class MaxFlowCalculationTargetSndLevelTransaction : public BaseTransaction {

public:
    typedef shared_ptr<MaxFlowCalculationTargetSndLevelTransaction> Shared;

public:
    MaxFlowCalculationTargetSndLevelTransaction(
        const NodeUUID &nodeUUID,
        MaxFlowCalculationTargetSndLevelMessage::Shared message,
        TrustLinesManager *manager,
        TopologyCacheManager *topologyCacheManager,
        Logger &logger,
        bool iAmGateway);

    MaxFlowCalculationTargetSndLevelMessage::Shared message() const;

    TransactionResult::SharedConst run();

protected:
    const string logHeader() const;

private:
    void sendResultToInitiator();

    void sendCachedResultToInitiator(
        TopologyCache::Shared maxFlowCalculationCachePtr);

    void sendGatewayResultToInitiator();

    void sendCachedGatewayResultToInitiator(
        TopologyCache::Shared maxFlowCalculationCachePtr);

private:
    MaxFlowCalculationTargetSndLevelMessage::Shared mMessage;
    TrustLinesManager *mTrustLinesManager;
    TopologyCacheManager *mTopologyCacheManager;
    bool mIAmGateway;
};


#endif //GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONTARGETSNDLEVELTRANSACTION_H
