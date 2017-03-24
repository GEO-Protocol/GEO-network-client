
#ifndef GEO_NETWORK_CLIENT_CORE_H
#define GEO_NETWORK_CLIENT_CORE_H

#include "common/Types.h"
#include "common/NodeUUID.h"

#include "settings/Settings.h"
#include "db/operations_history_storage/storage/OperationsHistoryStorage.h"
#include "network/communicator/Communicator.h"
#include "interface/commands_interface/interface/CommandsInterface.h"
#include "interface/results_interface/interface/ResultsInterface.h"
#include "trust_lines/manager/TrustLinesManager.h"
#include "transactions/manager/TransactionsManager.h"
#include "delayed_tasks/Cycles.h"
#include "max_flow_calculation/manager/MaxFlowCalculationTrustLineManager.h"
#include "max_flow_calculation/cashe/MaxFlowCalculationCacheManager.h"
#include "delayed_tasks/MaxFlowCalculationCacheUpdateDelayedTask.h"
#include "io/storage/StorageHandler.h"
#include "io/storage/RoutingTableHandler.h"

#include "logger/Logger.h"

#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include <boost/signals2.hpp>

using namespace std;

namespace as = boost::asio;
namespace signals = boost::signals2;
namespace history = db::operations_history_storage;

class Core {

public:
    Core();

    ~Core();

    int run();

private:
    int initCoreComponents();

    int initSettings();

    int initOperationsHistoryStorage();

    int initCommunicator(
        const json &conf);

    int initCommandsInterface();

    int initResultsInterface();

    int initTrustLinesManager();

    int initMaxFlowCalculationTrustLineManager();

    int initMaxFlowCalculationCacheManager();

    int initTransactionsManager();

    int initDelayedTasks();

    int initStorageHandler();

    void connectCommunicatorSignals();

    void connectTrustLinesManagerSignals();

    void connectDelayedTasksSignals();

    void connectSignalsToSlots();

    void onMessageReceivedSlot(
        Message::Shared message);

    void onMessageSendSlot(
        Message::Shared message,
        const NodeUUID &contractorUUID);

    void onTrustLineCreatedSlot(
        const NodeUUID &contractorUUID, 
        const TrustLineDirection direction);

    void onTrustLineStateModifiedSlot(
        const NodeUUID &contractorUUID,
        const TrustLineDirection direction);

    void onDelayedTaskCycleSixNodesSlot();

    void onDelayedTaskCycleFiveNodesSlot();

    void zeroPointers();

    void cleanupMemory();

    void JustToTestSomething();

    void onDelayedTaskMaxFlowCalculationCacheUpdateSlot();

    void writePIDFile();

    // TODO: remove after testing
    void testStorageHandler();

protected:
    Logger mLog;

    NodeUUID mNodeUUID;
    as::io_service mIOService;

    Settings *mSettings;
    history::OperationsHistoryStorage *mOperationsHistoryStorage;
    Communicator *mCommunicator;
    CommandsInterface *mCommandsInterface;
    ResultsInterface *mResultsInterface;
    TrustLinesManager *mTrustLinesManager;
    TransactionsManager *mTransactionsManager;
    CyclesDelayedTasks *mCyclesDelayedTasks;
    MaxFlowCalculationTrustLineManager *mMaxFlowCalculationTrustLimeManager;
    MaxFlowCalculationCacheManager *mMaxFlowCalculationCacheManager;
    MaxFlowCalculationCacheUpdateDelayedTask *mMaxFlowCalculationCacheUpdateDelayedTask;
    StorageHandler *mStorageHandler;
};

#endif //GEO_NETWORK_CLIENT_CORE_H
