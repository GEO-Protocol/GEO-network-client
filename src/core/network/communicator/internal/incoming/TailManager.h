//
// Created by minyor on 14.03.19.
//

#ifndef GEO_NETWORK_CLIENT_TAILMANAGER_H
#define GEO_NETWORK_CLIENT_TAILMANAGER_H


#include <list>
#include "../../../messages/Message.hpp"
#include "../../../../logger/Logger.h"

class TailManager {
public:
    TailManager(
        Logger &logger
    );
    ~TailManager();

public:
    typedef std::list<Message::Shared> Tail;

public:
    Tail &getFlowTail();

private:
    Tail mFlowTail;
    Logger &mLog;
};


#endif //GEO_NETWORK_CLIENT_TAILMANAGER_H
