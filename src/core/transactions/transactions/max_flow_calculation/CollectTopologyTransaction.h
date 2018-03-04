#ifndef GEO_NETWORK_CLIENT_COLLECTTOPOLOGYTRANSACTION_H
#define GEO_NETWORK_CLIENT_COLLECTTOPOLOGYTRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../topology/manager/TopologyTrustLinesManager.h"
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
        const SerializedEquivalent equivalent,
        const vector<NodeUUID> &contractors,
        TrustLinesManager *manager,
        TopologyTrustLinesManager *topologyTrustLineManager,
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
    TopologyTrustLinesManager *mTopologyTrustLineManager;
    TopologyCacheManager *mTopologyCacheManager;
    MaxFlowCacheManager *mMaxFlowCacheManager;

    vector<NodeUUID> mContractors;
};


#endif //GEO_NETWORK_CLIENT_COLLECTTOPOLOGYTRANSACTION_H
