#ifndef GEO_NETWORK_CLIENT_OUTGOINGMESSAGESHANDLER_H
#define GEO_NETWORK_CLIENT_OUTGOINGMESSAGESHANDLER_H

#include "OutgoingNodesHandler.h"

#include "../common/Types.h"


class OutgoingMessagesHandler {
public:
    OutgoingMessagesHandler(
        IOService &ioService,
        UDPSocket &socket,
        UUID2Address &uuid2AddressService,
        Logger &log)
        noexcept;

    void sendMessage(
        const Message::Shared message,
        const NodeUUID &addressee);

protected:
    Logger &mLog;
    OutgoingNodesHandler mNodes;
};


#endif //GEO_NETWORK_CLIENT_OUTGOINGMESSAGESHANDLER_H
