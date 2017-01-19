#ifndef GEO_NETWORK_CLIENT_CORE_H
#define GEO_NETWORK_CLIENT_CORE_H

#include "common/NodeUUID.h"

#include "settings/Settings.h"
#include "network/communicator/Communicator.h"
#include "interface/commands/interface/CommandsInterface.h"
#include "interface/results/interface/ResultsInterface.h"
#include "trust_lines/manager/TrustLinesManager.h"
#include "transactions/manager/TransactionsManager.h"

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

    int initSlots();

    void connectCommunicatorSignals();

    void connectSignalsToSlots();

    void zeroPointers();

    void cleanupMemory();

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

    NetworkSlots *networkSlots;

private:
    class NetworkSlots {

    public:
        NetworkSlots(
            Communicator *communicator,
            TransactionsManager *manager,
            Logger *logger);

        void onMessageReceivedSlot(
            Message::Shared message);

        void onMessageSendSlot(
            Message::Shared message,
            const NodeUUID &contractorUUID);

    private:
        Communicator *mCommunicator;
        TransactionsManager *mTransactionsManager;
        Logger *mLog;
    };
};

#endif //GEO_NETWORK_CLIENT_CORE_H
