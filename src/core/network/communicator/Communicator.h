#ifndef GEO_NETWORK_CLIENT_COMMUNICATOR_H
#define GEO_NETWORK_CLIENT_COMMUNICATOR_H

#include "../../common/Types.h"

#include "internal/common/Types.h"
#include "internal/outgoing/OutgoingMessagesHandler.h"
#include "internal/incoming/IncomingMessagesHandler.h"
#include "internal/queue/ConfirmationRequiredMessagesHandler.h"
#include "internal/uuid2address/UUID2Address.h"


using namespace std;
using namespace boost::asio::ip;


class Communicator {
public:
    signals::signal<void(Message::Shared)> signalMessageReceived;

public:
    explicit Communicator(
        IOService &ioService,
        const Host &interface,
        const Port port,
        const Host &uuid2AddressHost,
        const Port uuid2AddressPort,
        Logger &logger)
        noexcept(false);

    bool joinUUID2Address(
        const NodeUUID &nodeUUID)
        noexcept;

    void beginAcceptMessages()
        noexcept;

    void sendMessage (
        const Message::Shared kMessage,
        const NodeUUID &kContractorUUID)
        noexcept;

protected:
    /**
     * This slot fires up every time when new message was received from the network.
     */
    void onMessageReceived(
        Message::Shared message);

    /**
     * This slot fires up when next postponed message must be sent
     * from the confirmation messages queue.
     */
    void onConfirmationRequiredMessageReadyToResend(
        pair<NodeUUID, TransactionMessage::Shared>);



protected:
    const Host mInterface;
    const Port mPort;
    IOService &mIOService;
    Logger &mLog;

    unique_ptr<UDPSocket> mSocket;
    unique_ptr<UUID2Address> mUUID2AddressService;
    unique_ptr<IncomingMessagesHandler> mIncomingMessagesHandler;
    unique_ptr<OutgoingMessagesHandler> mOutgoingMessagesHandler;
    unique_ptr<ConfirmationRequiredMessagesHandler> mConfirmationRequiredMessagesHandler;
};


#endif //GEO_NETWORK_CLIENT_COMMUNICATOR_H