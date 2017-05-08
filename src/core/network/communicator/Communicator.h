#ifndef GEO_NETWORK_CLIENT_COMMUNICATOR_H
#define GEO_NETWORK_CLIENT_COMMUNICATOR_H

#include "../../common/Types.h"
#include "../../common/exceptions/RuntimeError.h"
#include "../../common/exceptions/NotFoundError.h"

#include "internal/common/Types.h"
#include "internal/outgoing/OutgoingMessagesHandler.h"
#include "internal/incoming/IncomingMessagesHandler.h"
#include "internal/uuid2address/UUID2Address.h"

#include <boost/asio.hpp>

#ifdef MAC_OS
#include <stdlib.h>
#endif

#ifdef LINUX
#include <malloc.h>
#endif


using namespace std;


class Communicator {
public:
    signals::signal<void(Message::Shared)> signalMessageReceived;

public:
    explicit Communicator(
        IOService &ioService,
        const string &interface,
        const uint16_t port,
        const string &uuid2AddressHost,
        const uint16_t uuid2AddressPort,
        Logger &logger);

    bool joinUUID2Address(
        const NodeUUID &nodeUUID)
        noexcept;

    void beginAcceptMessages()
        noexcept;

    void sendMessage (
        const Message::Shared kMessage,
        const NodeUUID &kContractorUUID)
        noexcept;

private:
    IOService &mIOService;
    const string mInterface;
    const uint16_t mPort;

    unique_ptr<UDPSocket> mSocket;
    unique_ptr<UUID2Address> mUUID2AddressService;
    unique_ptr<IncomingMessagesHandler> mIncomingMessagesHandler;
    unique_ptr<OutgoingMessagesHandler> mOutgoingMessagesHandler;

    Logger &mLog;
};


#endif //GEO_NETWORK_CLIENT_COMMUNICATOR_H
