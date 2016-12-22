#ifndef GEO_NETWORK_CLIENT_CORE_H
#define GEO_NETWORK_CLIENT_CORE_H

#include "settings/Settings.h"
#include "network/Communicator.h"
#include "interface/commands/interface/CommandsInterface.h"
#include "logger/Logger.h"

#include <boost/filesystem.hpp>

using namespace std;
namespace as = boost::asio;


class Core {
public:
    Core();
    ~Core();

    int run();

protected:
    NodeUUID mNodeUUID;
    Logger mLog;
    as::io_service mIOService;

    Settings *mSettings;
    Communicator *mCommunicator;
    CommandsInterface *mCommandsInterface;
    ResultsInterface *mResultsInterface;
    TransactionsManager *mTransactionsManager;

private:
    int initCoreComponents();
    int initSettings();
    int initCommandsInterface();
    int initResultsInterface();
    int initTransactionsManager();
    int initCommunicator(const json &conf);

    void zeroPointers();
    void cleanupMemory();
};


#endif //GEO_NETWORK_CLIENT_CORE_H
