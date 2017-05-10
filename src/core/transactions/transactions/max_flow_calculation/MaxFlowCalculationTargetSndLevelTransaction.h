#ifndef GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONTARGETSNDLEVELTRANSACTION_H
#define GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONTARGETSNDLEVELTRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../network/messages/max_flow_calculation/MaxFlowCalculationTargetSndLevelMessage.h"
#include "../../../network/messages/max_flow_calculation/ResultMaxFlowCalculationMessage.h"
#include "../../../max_flow_calculation/cashe/MaxFlowCalculationCacheManager.h"

class MaxFlowCalculationTargetSndLevelTransaction : public BaseTransaction {

public:
    typedef shared_ptr<MaxFlowCalculationTargetSndLevelTransaction> Shared;

public:
    MaxFlowCalculationTargetSndLevelTransaction(
        const NodeUUID &nodeUUID,
        MaxFlowCalculationTargetSndLevelMessage::Shared message,
        TrustLinesManager *manager,
        MaxFlowCalculationCacheManager *maxFlowCalculationCacheManager,
        Logger *logger);

    MaxFlowCalculationTargetSndLevelMessage::Shared message() const;

    TransactionResult::SharedConst run();

protected:
    const string logHeader() const;

private:
    void sendResultToInitiator();

    void sendCachedResultToInitiator(
        MaxFlowCalculationCache::Shared maxFlowCalculationCachePtr);

private:
    MaxFlowCalculationTargetSndLevelMessage::Shared mMessage;
    TrustLinesManager *mTrustLinesManager;
    MaxFlowCalculationCacheManager *mMaxFlowCalculationCacheManager;
};


#endif //GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONTARGETSNDLEVELTRANSACTION_H
