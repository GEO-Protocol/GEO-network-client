#ifndef GEO_NETWORK_CLIENT_COMMUNICATOR_H
#define GEO_NETWORK_CLIENT_COMMUNICATOR_H

#include "../../common/Types.h"

#include "internal/common/Types.h"
#include "internal/outgoing/OutgoingMessagesHandler.h"
#include "internal/incoming/IncomingMessagesHandler.h"
#include "internal/queue/ConfirmationRequiredMessagesHandler.h"
#include "internal/queue/ConfirmationNotStronglyRequiredMessagesHandler.h"
#include "internal/queue/ConfirmationResponseMessagesHandler.h"
#include "../../io/storage/StorageHandler.h"
#include "../../trust_lines/manager/TrustLinesManager.h"
#include "internal/uuid2address/UUID2Address.h"
#include <boost/asio/steady_timer.hpp>

#include "../../io/storage/CommunicatorStorageHandler.h"


using namespace std;
using namespace boost::asio::ip;


class Communicator {
public:
    signals::signal<void(Message::Shared)> signalMessageReceived;

    signals::signal<void(const SerializedEquivalent, const NodeUUID&)> signalClearTopologyCache;

public:
    explicit Communicator(
        IOService &ioService,
        const Host &interface,
        const Port port,
        const Host &uuid2AddressHost,
        const Port uuid2AddressPort,
        const NodeUUID &nodeUUID,
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

    void sendMessageWithCacheSaving (
        const TransactionMessage::Shared kMessage,
        const NodeUUID &kContractorUUID,
        Message::MessageType incomingMessageTypeFilter)
        noexcept;

    void processConfirmationMessage(
        ConfirmationMessage::Shared confirmationMessage);

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

    void onConfirmationNotStronglyRequiredMessageReadyToResend(
        pair<NodeUUID, MaxFlowCalculationConfirmationMessage::Shared>);

    void onClearTopologyCache(
        const SerializedEquivalent equivalent,
        const NodeUUID& nodeUUID);

    static string logHeader()
    noexcept;

    LoggerStream info() const
    noexcept;

    LoggerStream debug() const
    noexcept;

    LoggerStream error() const
    noexcept;

protected:
    const Host mInterface;
    const Port mPort;
    IOService &mIOService;
    unique_ptr<CommunicatorStorageHandler> mCommunicatorStorageHandler;
    NodeUUID mNodeUUID;
    Logger &mLog;

    unique_ptr<UDPSocket> mSocket;
    unique_ptr<UUID2Address> mUUID2AddressService;
    unique_ptr<IncomingMessagesHandler> mIncomingMessagesHandler;
    unique_ptr<OutgoingMessagesHandler> mOutgoingMessagesHandler;
    unique_ptr<ConfirmationRequiredMessagesHandler> mConfirmationRequiredMessagesHandler;
    unique_ptr<ConfirmationNotStronglyRequiredMessagesHandler> mConfirmationNotStronglyRequiredMessagesHandler;
    unique_ptr<ConfirmationResponseMessagesHandler> mConfirmationResponseMessagesHandler;
};


#endif //GEO_NETWORK_CLIENT_COMMUNICATOR_H
