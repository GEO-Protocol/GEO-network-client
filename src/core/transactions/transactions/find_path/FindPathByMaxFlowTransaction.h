#ifndef GEO_NETWORK_CLIENT_FINDPATHBYMAXFLOWTRANSACTION_H
#define GEO_NETWORK_CLIENT_FINDPATHBYMAXFLOWTRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../paths/PathsManager.h"
#include "../../../resources/manager/ResourcesManager.h"
#include "../../../resources/resources/PathsResource.h"

#include "../max_flow_calculation/CollectTopologyTransaction.h"

class FindPathByMaxFlowTransaction : public BaseTransaction {

public:
    typedef shared_ptr<FindPathByMaxFlowTransaction> Shared;

public:
    FindPathByMaxFlowTransaction(
        NodeUUID &nodeUUID,
        const NodeUUID &contractorUUID,
        const TransactionUUID &requestedTransactionUUID,
        PathsManager *pathsManager,
        ResourcesManager *resourcesManager,
        TrustLinesManager *manager,
        MaxFlowCalculationTrustLineManager *maxFlowCalculationTrustLineManager,
        MaxFlowCalculationCacheManager *maxFlowCalculationCacheManager,
        Logger &logger);

    TransactionResult::SharedConst run();

protected:
    const string logHeader() const;

protected:
    enum Stages {
        SendRequestForGettingRoutingTables = 1,
        BuildAllPaths
    };

private:
    // ToDo: move to separate config file
    const uint32_t kTopologyCollectingMilisecondsTimeout = 2000;

private:
    NodeUUID mContractorUUID;
    TransactionUUID mRequestedTransactionUUID;
    PathsManager *mPathsManager;
    ResourcesManager *mResourcesManager;
    TrustLinesManager *mTrustLineManager;
    MaxFlowCalculationTrustLineManager *mMaxFlowCalculationTrustLineManager;
    MaxFlowCalculationCacheManager *mMaxFlowCalculationCacheManager;
};


#endif //GEO_NETWORK_CLIENT_FINDPATHBYMAXFLOWTRANSACTION_H
