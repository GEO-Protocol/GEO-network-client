#ifndef GEO_NETWORK_CLIENT_CORE_H
#define GEO_NETWORK_CLIENT_CORE_H

#include "settings/Settings.h"
#include "network/Communicator.h"
#include "interface/CommandsInterface.h"
#include "logger/Logger.h"

#include <boost/filesystem.hpp>

using namespace std;
namespace as = boost::asio;


class Core {
public:
    Core();
    ~Core();

    int run();

private:
    int initCoreComponents();
    int initSettings();
//    int initCommandsAPI();
//    int initCommandsInterface();
    int initCommunicator(const json &conf);

    void zeroPointers();
    void cleanupMemory();

private:
    Logger mLog;

    as::io_service mIOService;

    Settings *mSettings;
    Communicator *mCommunicator;
//    CommandsAPI *mCommandsAPI;
//    CommandsInterface *mCommandsInterface;
};


#endif //GEO_NETWORK_CLIENT_CORE_H
