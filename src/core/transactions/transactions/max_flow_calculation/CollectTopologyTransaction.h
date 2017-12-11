#ifndef GEO_NETWORK_CLIENT_COLLECTTOPOLOGYTRANSACTION_H
#define GEO_NETWORK_CLIENT_COLLECTTOPOLOGYTRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../max_flow_calculation/manager/MaxFlowCalculationTrustLineManager.h"
#include "../../../max_flow_calculation/cashe/MaxFlowCalculationCacheManager.h"
#include "../../../max_flow_calculation/cashe/MaxFlowCalculationNodeCacheManager.h"

#include "../../../network/messages/max_flow_calculation/InitiateMaxFlowCalculationMessage.h"
#include "../../../network/messages/max_flow_calculation/MaxFlowCalculationSourceFstLevelMessage.h"

class CollectTopologyTransaction : public BaseTransaction {

public:
    typedef shared_ptr<CollectTopologyTransaction> Shared;

public:
    CollectTopologyTransaction(
        const NodeUUID &nodeUUID,
        const vector<NodeUUID> &contractors,
        TrustLinesManager *manager,
        MaxFlowCalculationTrustLineManager *maxFlowCalculationTrustLineManager,
        MaxFlowCalculationCacheManager *maxFlowCalculationCacheManager,
        MaxFlowCalculationNodeCacheManager *maxFlowCalculationNodeCacheManager,
        Logger &logger);

    TransactionResult::SharedConst run();

protected:
    const string logHeader() const;

private:
    void sendMessagesToContractors();

    void sendMessagesOnFirstLevel();

private:
    TrustLinesManager *mTrustLinesManager;
    MaxFlowCalculationTrustLineManager *mMaxFlowCalculationTrustLineManager;
    MaxFlowCalculationCacheManager *mMaxFlowCalculationCacheManager;
    MaxFlowCalculationNodeCacheManager *mMaxFlowCalculationNodeCacheManager;

    vector<NodeUUID> mContractors;
};


#endif //GEO_NETWORK_CLIENT_COLLECTTOPOLOGYTRANSACTION_H
