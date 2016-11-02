//
// Created by hsc on 31.10.16.
//

#ifndef GEO_NETWORK_CLIENT_COMMUNICATOR_H
#define GEO_NETWORK_CLIENT_COMMUNICATOR_H

#include "internal/OutgoingMessagesHandler.h"

class Communicator {
public:
    explicit Communicator();

    OutgoingMessagesHandler* outgoingMessagesHandler() const;

private:
    OutgoingMessagesHandler *mOutgoingMessagesHandler;
};


#endif //GEO_NETWORK_CLIENT_COMMUNICATOR_H
