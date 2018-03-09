#ifndef GEO_NETWORK_CLIENT_EQUIVALENTSSUBSYSTEMSROUTER_H
#define GEO_NETWORK_CLIENT_EQUIVALENTSSUBSYSTEMSROUTER_H

#include "../trust_lines/manager/TrustLinesManager.h"
#include "../topology/manager/TopologyTrustLinesManager.h"
#include "../topology/cashe/TopologyCacheManager.h"
#include "../delayed_tasks/TopologyCacheUpdateDelayedTask.h"
#include "../topology/cashe/MaxFlowCacheManager.h"
#include "../paths/PathsManager.h"
#include "../delayed_tasks/NotifyThatIAmIsGatewayDelayedTask.h"

#include <map>

namespace as = boost::asio;
namespace signals = boost::signals2;

class EquivalentsSubsystemsRouter {

public:
    typedef signals::signal<void()> GatewayNotificationSignal;

public:
    EquivalentsSubsystemsRouter(
        NodeUUID &nodeUUID,
        StorageHandler *storageHandler,
        as::io_service &ioService,
        vector<SerializedEquivalent> &equivalentsIAmGateway,
        Logger &logger);

    vector<SerializedEquivalent> equivalents() const;

    bool iAmGateway(
        const SerializedEquivalent equivalent) const;

    TrustLinesManager* trustLinesManager(
        const SerializedEquivalent equivalent) const;

    TopologyTrustLinesManager* topologyTrustLineManager(
        const SerializedEquivalent equivalent) const;

    TopologyCacheManager* topologyCacheManager(
        const SerializedEquivalent equivalent) const;

    MaxFlowCacheManager* maxFlowCacheManager(
        const SerializedEquivalent equivalent) const;

    PathsManager* pathsManager(
        const SerializedEquivalent equivalent) const;

    void initNewEquivalent(
        const SerializedEquivalent equivalent);

public:
    mutable GatewayNotificationSignal gatewayNotificationSignal;

protected:
    string logHeader() const;

    LoggerStream warning() const;

    LoggerStream error() const;

    LoggerStream info() const;

    LoggerStream debug() const;

private:
    void subscribeForGatewayNotification(
        NotifyThatIAmIsGatewayDelayedTask::GatewayNotificationSignal &signal);

    void onGatewayNotificationSlot();

private:
    NodeUUID mNodeUUID;
    map<SerializedEquivalent, bool> mIAmGateways;
    as::io_service &mIOService;
    StorageHandler *mStorageHandler;
    Logger &mLogger;
    vector<SerializedEquivalent> mEquivalents;
    map<SerializedEquivalent, unique_ptr<TrustLinesManager>> mTrustLinesManagers;
    map<SerializedEquivalent, unique_ptr<TopologyTrustLinesManager>> mTopologyTrustLinesManagers;
    map<SerializedEquivalent, unique_ptr<TopologyCacheManager>> mTopologyCacheManagers;
    map<SerializedEquivalent, unique_ptr<TopologyCacheUpdateDelayedTask>> mTopologyCacheUpdateDelayedTasks;
    map<SerializedEquivalent, unique_ptr<MaxFlowCacheManager>> mMaxFlowCacheManagers;
    map<SerializedEquivalent, unique_ptr<PathsManager>> mPathsManagers;
    unique_ptr<NotifyThatIAmIsGatewayDelayedTask> mNotifyThatIAmIsGatewayDelayedTask;
};


#endif //GEO_NETWORK_CLIENT_EQUIVALENTSSUBSYSTEMSROUTER_H
