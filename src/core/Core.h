
#ifndef GEO_NETWORK_CLIENT_CORE_H
#define GEO_NETWORK_CLIENT_CORE_H

#include "common/Types.h"
#include "common/NodeUUID.h"

#include "settings/Settings.h"
#include "network/communicator/Communicator.h"
#include "interface/commands_interface/interface/CommandsInterface.h"
#include "interface/results_interface/interface/ResultsInterface.h"
#include "trust_lines/manager/TrustLinesManager.h"
#include "transactions/manager/TransactionsManager.h"
#include "delayed_tasks/Cycles.h"
#include "logger/Logger.h"

#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include <boost/signals2.hpp>

using namespace std;

namespace as = boost::asio;
namespace signals = boost::signals2;

class Core {

public:
    Core();

    ~Core();

    int run();

private:
    int initCoreComponents();

    int initSettings();

    int initCommunicator(
        const json &conf);

    int initCommandsInterface();

    int initResultsInterface();

    int initTrustLinesManager();

    int initTransactionsManager();

    int initDelayedTasks();

    void connectCommunicatorSignals();

    void connectTrustLinesManagerSignals();

    void connectSignalsToSlots();

    void connectDelayedTasksSignals();

    void onDelayedTaskCycleSixNodesSlot();

    void onDelayedTaskCycleFiveNodesSlot();

    void onMessageReceivedSlot(
        Message::Shared message);

    void onMessageSendSlot(
        Message::Shared message,
        const NodeUUID &contractorUUID);

    void onTrustLineCreatedSlot(
        const NodeUUID &contractorUUID,
        const TrustLineDirection direction);

    void zeroPointers();

    void cleanupMemory();

    void JustToTestSomething();
protected:
    Logger mLog;

    NodeUUID mNodeUUID;
    as::io_service mIOService;

    Settings *mSettings;
    Communicator *mCommunicator;
    CommandsInterface *mCommandsInterface;
    ResultsInterface *mResultsInterface;
    TrustLinesManager *mTrustLinesManager;
    TransactionsManager *mTransactionsManager;
    CyclesDelayedTasks *mCyclesDelayedTasks;
};

#endif //GEO_NETWORK_CLIENT_CORE_H
