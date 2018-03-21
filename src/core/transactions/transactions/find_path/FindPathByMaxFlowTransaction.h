#ifndef GEO_NETWORK_CLIENT_FINDPATHBYMAXFLOWTRANSACTION_H
#define GEO_NETWORK_CLIENT_FINDPATHBYMAXFLOWTRANSACTION_H

#include "../base/BaseCollectTopologyTransaction.h"
#include "../../../paths/PathsManager.h"
#include "../../../resources/manager/ResourcesManager.h"
#include "../../../resources/resources/PathsResource.h"

#include "../max_flow_calculation/CollectTopologyTransaction.h"

class FindPathByMaxFlowTransaction : public BaseCollectTopologyTransaction {

public:
    typedef shared_ptr<FindPathByMaxFlowTransaction> Shared;

public:
    FindPathByMaxFlowTransaction(
        NodeUUID &nodeUUID,
        const NodeUUID &contractorUUID,
        const TransactionUUID &requestedTransactionUUID,
        const SerializedEquivalent equivalent,
        PathsManager *pathsManager,
        ResourcesManager *resourcesManager,
        TrustLinesManager *manager,
        TopologyTrustLinesManager *topologyTrustLineManager,
        TopologyCacheManager *topologyCacheManager,
        MaxFlowCacheManager *maxFlowCacheManager,
        Logger &logger);

protected:
    const string logHeader() const;

private:
    TransactionResult::SharedConst sendRequestForCollectingTopology();

    TransactionResult::SharedConst processCollectingTopologyShortly(){}

    TransactionResult::SharedConst processCollectingTopology();

private:
    // ToDo: move to separate config file
    static const uint32_t kTopologyCollectingMillisecondsTimeout = 3000;

    static const uint32_t kTopologyCollectingAgainMillisecondsTimeout = 500;
    static const uint32_t kMaxTopologyCollectingMillisecondsTimeout = 6000;
    static const uint16_t kCountRunningProcessCollectingTopologyStage =
            (kMaxTopologyCollectingMillisecondsTimeout - kTopologyCollectingMillisecondsTimeout) /
            kTopologyCollectingAgainMillisecondsTimeout;

private:
    NodeUUID mContractorUUID;
    TransactionUUID mRequestedTransactionUUID;
    PathsManager *mPathsManager;
    ResourcesManager *mResourcesManager;
    size_t mCountProcessCollectingTopologyRun;
};


#endif //GEO_NETWORK_CLIENT_FINDPATHBYMAXFLOWTRANSACTION_H
