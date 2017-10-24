#ifndef GEO_NETWORK_CLIENT_BASECOLLECTTOPOLOGYTRANSACTION_H
#define GEO_NETWORK_CLIENT_BASECOLLECTTOPOLOGYTRANSACTION_H

#include "BaseTransaction.h"

#include "../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../max_flow_calculation/manager/MaxFlowCalculationTrustLineManager.h"
#include "../../../max_flow_calculation/cashe/MaxFlowCalculationCacheManager.h"

#include "../../../network/messages/max_flow_calculation/ResultMaxFlowCalculationMessage.h"
#include "../../../network/messages/max_flow_calculation/ResultMaxFlowCalculationGatewayMessage.h"

class BaseCollectTopologyTransaction : public BaseTransaction {

public:
    typedef shared_ptr<BaseCollectTopologyTransaction> Shared;

public:
    BaseCollectTopologyTransaction(
        const TransactionType type,
        NodeUUID &nodeUUID,
        TrustLinesManager *trustLinesManager,
        MaxFlowCalculationTrustLineManager *maxFlowCalculationTrustLineManager,
        MaxFlowCalculationCacheManager *maxFlowCalculationCacheManager,
        Logger &logger);

    TransactionResult::SharedConst run();

protected:
    enum Stages {
        SendRequestForCollectingTopology = 1,
        ProcessCollectingTopology
    };

protected:
    virtual TransactionResult::SharedConst sendRequestForCollectingTopology() = 0;

    virtual TransactionResult::SharedConst processCollectingTopology() = 0;

    void fillTopology();

protected:
    TrustLinesManager *mTrustLinesManager;
    MaxFlowCalculationTrustLineManager *mMaxFlowCalculationTrustLineManager;
    MaxFlowCalculationCacheManager *mMaxFlowCalculationCacheManager;

};


#endif //GEO_NETWORK_CLIENT_BASECOLLECTTOPOLOGYTRANSACTION_H
