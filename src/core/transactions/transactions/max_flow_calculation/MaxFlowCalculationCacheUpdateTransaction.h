#ifndef GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONCACHEUPDATETRANSACTION_H
#define GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONCACHEUPDATETRANSACTION_H

#include "../../../logger/Logger.h"
#include "../base/BaseTransaction.h"
#include "../../../max_flow_calculation/cashe/MaxFlowCalculationCacheManager.h"
#include "../../../max_flow_calculation/manager/MaxFlowCalculationTrustLineManager.h"

class MaxFlowCalculationCacheUpdateTransaction : public BaseTransaction {

public:
    typedef shared_ptr<MaxFlowCalculationCacheUpdateTransaction> Shared;

public:

    MaxFlowCalculationCacheUpdateTransaction(
        NodeUUID &nodeUUID,
        MaxFlowCalculationCacheManager *maxFlowCalculationCacheManager,
        MaxFlowCalculationTrustLineManager *maxFlowCalculationTrustLineManager,
        Logger *logger);

    TransactionResult::SharedConst run();

protected:
    const string logHeader() const;

private:

    MaxFlowCalculationCacheManager *mMaxFlowCalculationCacheManager;
    MaxFlowCalculationTrustLineManager *mMaxFlowCalculationTrustLineManager;

};


#endif //GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONCACHEUPDATETRANSACTION_H
