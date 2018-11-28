#ifndef GEO_NETWORK_CLIENT_COMMUNICATOR_H
#define GEO_NETWORK_CLIENT_COMMUNICATOR_H

#include "../../common/Types.h"

#include "internal/common/Types.h"
#include "internal/outgoing/OutgoingMessagesHandler.h"
#include "internal/incoming/IncomingMessagesHandler.h"
#include "internal/queue/ConfirmationRequiredMessagesHandler.h"
#include "internal/queue/ConfirmationNotStronglyRequiredMessagesHandler.h"
#include "internal/queue/ConfirmationResponseMessagesHandler.h"
#include "internal/queue/PingMessagesHandler.h"
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
        ContractorsManager *contractorsManager,
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

    void sendMessage (
        const Message::Shared kMessage,
        const ContractorID contractorID)
        noexcept;

    void sendMessageWithCacheSaving (
        const TransactionMessage::Shared kMessage,
        const NodeUUID &kContractorUUID,
        Message::MessageType incomingMessageTypeFilter,
        uint32_t cacheLivingTime)
        noexcept;

    void processConfirmationMessage(
        ConfirmationMessage::Shared confirmationMessage);

    void processPongMessage(
        ContractorID contractorID);

    void enqueueContractorWithPostponedSending(
        ContractorID contractorID);

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

    void onPingMessageReadyToResend(
        pair<ContractorID, PingMessage::Shared>);

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
    ContractorsManager *mContractorsManager;
    unique_ptr<IncomingMessagesHandler> mIncomingMessagesHandler;
    unique_ptr<OutgoingMessagesHandler> mOutgoingMessagesHandler;
    unique_ptr<ConfirmationRequiredMessagesHandler> mConfirmationRequiredMessagesHandler;
    unique_ptr<ConfirmationNotStronglyRequiredMessagesHandler> mConfirmationNotStronglyRequiredMessagesHandler;
    unique_ptr<ConfirmationResponseMessagesHandler> mConfirmationResponseMessagesHandler;
    unique_ptr<PingMessagesHandler> mPingMessagesHandler;
};


#endif //GEO_NETWORK_CLIENT_COMMUNICATOR_H
