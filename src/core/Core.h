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

#include <boost/filesystem.hpp>

using namespace std;
namespace as = boost::asio;

class Communicator;
class CommandsInterface;
class TransactionsManager;
class Core {

public:
    Core();

    ~Core();

    TransactionsManager* transactionsManager();

    int run();

protected:
    NodeUUID mNodeUUID;
    Logger mLog;
    as::io_service mIOService;

    Settings *mSettings;
    Communicator *mCommunicator;
    CommandsInterface *mCommandsInterface;
    ResultsInterface *mResultsInterface;
    TrustLinesManager *mTrustLinesManager;
    TransactionsManager *mTransactionsManager;


private:
    int initCoreComponents();
    int initSettings();
    int initCommandsInterface();
    int initResultsInterface();
    int initTrustLinesManager();
    int initTransactionsManager();
    int initCommunicator(const json &conf);

    void zeroPointers();
    void cleanupMemory();
};


#endif //GEO_NETWORK_CLIENT_CORE_H
