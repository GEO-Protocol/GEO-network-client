#ifndef GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONSOURCESNDLEVELTRANSACTION_H
#define GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONSOURCESNDLEVELTRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../network/messages/max_flow_calculation/MaxFlowCalculationSourceSndLevelMessage.h"
#include "../../../network/messages/max_flow_calculation/ResultMaxFlowCalculationMessage.h"
#include "../../../network/messages/max_flow_calculation/ResultMaxFlowCalculationGatewayMessage.h"
#include "../../../topology/cashe/TopologyCacheManager.h"

class MaxFlowCalculationSourceSndLevelTransaction : public BaseTransaction {

public:
    typedef shared_ptr<MaxFlowCalculationSourceSndLevelTransaction> Shared;

public:
    MaxFlowCalculationSourceSndLevelTransaction(
        const NodeUUID &nodeUUID,
        MaxFlowCalculationSourceSndLevelMessage::Shared message,
        TrustLinesManager *manager,
        TopologyCacheManager *topologyCacheManager,
        Logger &logger,
        bool iAmGateway);

    MaxFlowCalculationSourceSndLevelMessage::Shared message() const;

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
    MaxFlowCalculationSourceSndLevelMessage::Shared mMessage;
    TrustLinesManager *mTrustLinesManager;
    TopologyCacheManager *mTopologyCacheManager;
    bool mIAmGateway;
};


#endif //GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONSOURCESNDLEVELTRANSACTION_H
