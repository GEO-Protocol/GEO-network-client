
#ifndef GEO_NETWORK_CLIENT_CORE_H
#define GEO_NETWORK_CLIENT_CORE_H

#include "common/Types.h"
#include "common/NodeUUID.h"

#include "settings/Settings.h"
#include "network/communicator/Communicator.h"
#include "interface/commands_interface/interface/CommandsInterface.h"
#include "interface/results_interface/interface/ResultsInterface.h"
#include "trust_lines/manager/TrustLinesManager.h"
#include "resources/manager/ResourcesManager.h"
#include "transactions/manager/TransactionsManager.h"
#include "delayed_tasks/Cycles.h"
#include "max_flow_calculation/manager/MaxFlowCalculationTrustLineManager.h"
#include "max_flow_calculation/cashe/MaxFlowCalculationCacheManager.h"
#include "delayed_tasks/MaxFlowCalculationCacheUpdateDelayedTask.h"
#include "io/storage/StorageHandler.h"
#include "paths/PathsManager.h"

#include "logger/Logger.h"

#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include <boost/signals2.hpp>




#include "network/messages/debug/DebugMessage.h"


#include <sys/prctl.h>


using namespace std;

namespace as = boost::asio;
namespace signals = boost::signals2;

class Core {

public:
    Core(
        char* pArgv)
        noexcept;

    ~Core();

    int run();

private:
    int initSubsystems();

    int initSettings();

    int initLogger(const json &conf);

    int initCommunicator(
        const json &conf);

    int initCommandsInterface();

    int initResultsInterface();

    int initTrustLinesManager();

    int initMaxFlowCalculationTrustLineManager();

    int initMaxFlowCalculationCacheManager();

    int initResourcesManager();

    int initTransactionsManager();

    int initDelayedTasks();

    int initStorageHandler();

    int initPathsManager();

    void connectCommunicatorSignals();

    void connectCommandsInterfaceSignals();

    void connectTrustLinesManagerSignals();

    void connectDelayedTasksSignals();

    void connectResourcesManagerSignals();

    void connectSignalsToSlots();

    void onCommandReceivedSlot(
        BaseUserCommand::Shared command);

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

    void onDelayedTaskCycleThreeNodesSlot();

    void onDelayedTaskCycleFourNodesSlot();

    void onPathsResourceRequestedSlot(
        const TransactionUUID &transactionUUID,
        const NodeUUID &destinationNodeUUID);

    void onResourceCollectedSlot(
        BaseResource::Shared resource);

    void writePIDFile();

    void updateProcessName();

    void checkSomething();

    void test_FourNodesTransaction();

    void test_ThreeNodesTransaction();

    void printRTs();

protected:
    // This pointer is used to modify executable command description.
    // By default, it would point to the standard argv[0] char sequence;
    char* mCommandDescriptionPtr;

    NodeUUID mNodeUUID;
    as::io_service mIOService;

    unique_ptr<Logger> mLog;
    unique_ptr<Settings> mSettings;
    unique_ptr<Communicator> mCommunicator;
    unique_ptr<CommandsInterface> mCommandsInterface;
    unique_ptr<ResultsInterface> mResultsInterface;
    unique_ptr<TrustLinesManager> mTrustLinesManager;
    unique_ptr<ResourcesManager> mResourcesManager;
    unique_ptr<TransactionsManager> mTransactionsManager;
    unique_ptr<CyclesDelayedTasks> mCyclesDelayedTasks;
    unique_ptr<MaxFlowCalculationTrustLineManager> mMaxFlowCalculationTrustLimeManager;
    unique_ptr<MaxFlowCalculationCacheManager> mMaxFlowCalculationCacheManager;
    unique_ptr<MaxFlowCalculationCacheUpdateDelayedTask> mMaxFlowCalculationCacheUpdateDelayedTask;
    unique_ptr<StorageHandler> mStorageHandler;
    unique_ptr<PathsManager> mPathsManager;
};

#endif //GEO_NETWORK_CLIENT_CORE_H
