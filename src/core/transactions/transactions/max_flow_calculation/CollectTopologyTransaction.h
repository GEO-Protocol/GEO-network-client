#ifndef GEO_NETWORK_CLIENT_COLLECTTOPOLOGYTRANSACTION_H
#define GEO_NETWORK_CLIENT_COLLECTTOPOLOGYTRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../topology/manager/TopologyTrustLineManager.h"
#include "../../../topology/cashe/TopologyCacheManager.h"
#include "../../../topology/cashe/MaxFlowCacheManager.h"

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
        TopologyTrustLineManager *topologyTrustLineManager,
        TopologyCacheManager *topologyCacheManager,
        MaxFlowCacheManager *maxFlowCacheManager,
        Logger &logger);

    TransactionResult::SharedConst run();

protected:
    const string logHeader() const;

private:
    void sendMessagesToContractors();

    void sendMessagesOnFirstLevel();

private:
    TrustLinesManager *mTrustLinesManager;
    TopologyTrustLineManager *mTopologyTrustLineManager;
    TopologyCacheManager *mTopologyCacheManager;
    MaxFlowCacheManager *mMaxFlowCacheManager;

    vector<NodeUUID> mContractors;
};


#endif //GEO_NETWORK_CLIENT_COLLECTTOPOLOGYTRANSACTION_H
