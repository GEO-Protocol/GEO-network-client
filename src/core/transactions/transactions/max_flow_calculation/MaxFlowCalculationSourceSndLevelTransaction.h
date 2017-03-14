#ifndef GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONSOURCESNDLEVELTRANSACTION_H
#define GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONSOURCESNDLEVELTRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../network/messages/max_flow_calculation/MaxFlowCalculationSourceSndLevelMessage.h"
#include "../../../network/messages/max_flow_calculation/ResultMaxFlowCalculationMessage.h"
#include "../../../max_flow_calculation/cashe/MaxFlowCalculationCacheManager.h"

class MaxFlowCalculationSourceSndLevelTransaction : public BaseTransaction {

public:
    typedef shared_ptr<MaxFlowCalculationSourceSndLevelTransaction> Shared;

public:
    MaxFlowCalculationSourceSndLevelTransaction(
        const NodeUUID &nodeUUID,
        MaxFlowCalculationSourceSndLevelMessage::Shared message,
        TrustLinesManager *manager,
        MaxFlowCalculationCacheManager *maxFlowCalculationCacheManager,
        Logger *logger);

    MaxFlowCalculationSourceSndLevelMessage::Shared message() const;

    TransactionResult::SharedConst run();

private:

    void sendResultToInitiator();

    void sendCachedResultToInitiator(
        MaxFlowCalculationCache::Shared maxFlowCalculationCachePtr);

private:
    MaxFlowCalculationSourceSndLevelMessage::Shared mMessage;
    TrustLinesManager *mTrustLinesManager;
    MaxFlowCalculationCacheManager *mMaxFlowCalculationCacheManager;
    Logger *mLog;

};


#endif //GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONSOURCESNDLEVELTRANSACTION_H
