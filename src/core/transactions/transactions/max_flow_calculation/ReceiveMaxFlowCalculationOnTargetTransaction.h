#ifndef GEO_NETWORK_CLIENT_RECEIVEMAXFLOWCALCULATIONTRANSACTION_H
#define GEO_NETWORK_CLIENT_RECEIVEMAXFLOWCALCULATIONTRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../contractors/ContractorsManager.h"
#include "../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../network/messages/max_flow_calculation/InitiateMaxFlowCalculationMessage.h"
#include "../../../network/messages/max_flow_calculation/ResultMaxFlowCalculationMessage.h"
#include "../../../network/messages/max_flow_calculation/MaxFlowCalculationTargetFstLevelMessage.h"
#include "../../../topology/cashe/TopologyCacheManager.h"

class ReceiveMaxFlowCalculationOnTargetTransaction : public BaseTransaction {

public:
    typedef shared_ptr<ReceiveMaxFlowCalculationOnTargetTransaction> Shared;

public:
    ReceiveMaxFlowCalculationOnTargetTransaction(
        const NodeUUID &nodeUUID,
        InitiateMaxFlowCalculationMessage::Shared message,
        ContractorsManager *contractorsManager,
        TrustLinesManager *trustLinesManager,
        TopologyCacheManager *topologyCacheManager,
        Logger &logger);

    TransactionResult::SharedConst run();

protected:
    const string logHeader() const;

private:
    void sendMessagesOnFirstLevel();

    void sendResultToInitiator();

    void sendCachedResultToInitiator(
        TopologyCache::Shared maxFlowCalculationCachePtr,
        TopologyCacheNew::Shared maxFlowCalculationCachePtrNew);

private:
    InitiateMaxFlowCalculationMessage::Shared mMessage;
    ContractorsManager *mContractorsManager;
    TrustLinesManager *mTrustLinesManager;
    TopologyCacheManager *mTopologyCacheManager;
};


#endif //GEO_NETWORK_CLIENT_RECEIVEMAXFLOWCALCULATIONTRANSACTION_H
