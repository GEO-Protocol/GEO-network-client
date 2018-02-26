#ifndef GEO_NETWORK_CLIENT_RECEIVEMAXFLOWCALCULATIONTRANSACTION_H
#define GEO_NETWORK_CLIENT_RECEIVEMAXFLOWCALCULATIONTRANSACTION_H

#include "../base/BaseTransaction.h"
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
        TrustLinesManager *trustLinesManager,
        TopologyCacheManager *topologyCacheManager,
        Logger &logger);

    InitiateMaxFlowCalculationMessage::Shared message() const;

    TransactionResult::SharedConst run();

protected:
    const string logHeader() const;

private:
    void sendMessagesOnFirstLevel();

    void sendResultToInitiator();

    void sendCachedResultToInitiator(
        TopologyCache::Shared maxFlowCalculationCachePtr);

private:
    InitiateMaxFlowCalculationMessage::Shared mMessage;
    TrustLinesManager *mTrustLinesManager;
    TopologyCacheManager *mTopologyCacheManager;
};


#endif //GEO_NETWORK_CLIENT_RECEIVEMAXFLOWCALCULATIONTRANSACTION_H
