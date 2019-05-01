#ifndef GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONTARGETSNDLEVELTRANSACTION_H
#define GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONTARGETSNDLEVELTRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../contractors/ContractorsManager.h"
#include "../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../topology/cashe/TopologyCacheManager.h"
#include "../../../network/messages/max_flow_calculation/MaxFlowCalculationTargetSndLevelMessage.h"
#include "../../../network/messages/max_flow_calculation/ResultMaxFlowCalculationMessage.h"
#include "../../../network/messages/max_flow_calculation/ResultMaxFlowCalculationGatewayMessage.h"

class MaxFlowCalculationTargetSndLevelTransaction : public BaseTransaction {

public:
    typedef shared_ptr<MaxFlowCalculationTargetSndLevelTransaction> Shared;

public:
    MaxFlowCalculationTargetSndLevelTransaction(
        MaxFlowCalculationTargetSndLevelMessage::Shared message,
        ContractorsManager *contractorsManager,
        TrustLinesManager *manager,
        TopologyCacheManager *topologyCacheManager,
        Logger &logger,
        bool iAmGateway);

    TransactionResult::SharedConst run() override;

protected:
    const string logHeader() const override;

private:
    void sendResultToInitiator();

    void sendCachedResultToInitiator(
        TopologyCache::Shared maxFlowCalculationCachePtr);

    void sendGatewayResultToInitiator();

    void sendCachedGatewayResultToInitiator(
        TopologyCache::Shared maxFlowCalculationCachePtr);

private:
    MaxFlowCalculationTargetSndLevelMessage::Shared mMessage;
    ContractorsManager *mContractorsManager;
    TrustLinesManager *mTrustLinesManager;
    TopologyCacheManager *mTopologyCacheManager;
    bool mIAmGateway;
};


#endif //GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONTARGETSNDLEVELTRANSACTION_H
