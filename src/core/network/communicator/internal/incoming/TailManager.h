//
// Created by minyor on 14.03.19.
//

#ifndef GEO_NETWORK_CLIENT_TAILMANAGER_H
#define GEO_NETWORK_CLIENT_TAILMANAGER_H


#include <map>
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
    Tail &getTail(Message::MessageType type);

private:
    std::map<Message::MessageType, Tail> mTails;
    Logger &mLog;
};


#endif //GEO_NETWORK_CLIENT_TAILMANAGER_H
