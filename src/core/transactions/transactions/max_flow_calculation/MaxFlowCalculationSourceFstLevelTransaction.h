#ifndef GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONSOURCEFSTLEVELTRANSACTION_H
#define GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONSOURCEFSTLEVELTRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../max_flow_calculation/cashe/MaxFlowCalculationCacheManager.h"
#include "../../../network/messages/max_flow_calculation/MaxFlowCalculationSourceFstLevelMessage.h"
#include "../../../network/messages/max_flow_calculation/MaxFlowCalculationSourceSndLevelMessage.h"
#include "../../../network/messages/max_flow_calculation/ResultMaxFlowCalculationMessage.h"

class MaxFlowCalculationSourceFstLevelTransaction : public BaseTransaction  {

public:
    typedef shared_ptr<MaxFlowCalculationSourceFstLevelTransaction> Shared;

public:
    MaxFlowCalculationSourceFstLevelTransaction(
        const NodeUUID &nodeUUID,
        MaxFlowCalculationSourceFstLevelMessage::Shared message,
        TrustLinesManager *trustLinesManager,
        MaxFlowCalculationCacheManager *maxFlowCalculationCacheManager,
        Logger &logger,
        bool iAmGateway);

    MaxFlowCalculationSourceFstLevelMessage::Shared message() const;

    TransactionResult::SharedConst run();

private:
    void sendResultToInitiator();

    void sendCachedResultToInitiator(
            MaxFlowCalculationCache::Shared maxFlowCalculationCachePtr);

protected:
    const string logHeader() const;

private:
    MaxFlowCalculationSourceFstLevelMessage::Shared mMessage;
    TrustLinesManager *mTrustLinesManager;
    MaxFlowCalculationCacheManager *mMaxFlowCalculationCacheManager;
    bool mIAmGateway;
};


#endif //GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONSOURCEFSTLEVELTRANSACTION_H
