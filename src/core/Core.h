#ifndef GEO_NETWORK_CLIENT_CORE_H
#define GEO_NETWORK_CLIENT_CORE_H

#include "network/Communicator.h"

#include <boost/filesystem.hpp>

#include <iostream>

class Core {
public:
    Core();

    int run();

private:
    Communicator *mCommunicator;
};


#endif //GEO_NETWORK_CLIENT_CORE_H
