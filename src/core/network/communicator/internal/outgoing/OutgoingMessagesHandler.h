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
        ContractorsManager *contractorsManager,
        Logger &log)
        noexcept;

    void sendMessage(
        const Message::Shared message,
        const NodeUUID &addressee);

    void sendMessage(
        const Message::Shared message,
        const ContractorID addressee);

protected:
    Logger &mLog;
    OutgoingNodesHandler mNodes;
};


#endif //GEO_NETWORK_CLIENT_OUTGOINGMESSAGESHANDLER_H
