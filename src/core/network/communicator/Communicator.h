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
#include <boost/asio/steady_timer.hpp>

#include "../../io/storage/CommunicatorStorageHandler.h"


using namespace std;
using namespace boost::asio::ip;


class Communicator {
public:
    signals::signal<void(Message::Shared)> signalMessageReceived;

    signals::signal<void(const SerializedEquivalent, BaseAddress::Shared)> signalClearTopologyCache;

public:
    explicit Communicator(
        IOService &ioService,
        ContractorsManager *contractorsManager,
        TailManager &tailManager,
        Logger &logger)
        noexcept(false);

    void beginAcceptMessages()
        noexcept;

    void sendMessage (
        const Message::Shared kMessage,
        const ContractorID contractorID)
        noexcept;

    void sendMessage(
        const Message::Shared kMessage,
        const BaseAddress::Shared contractorAddress)
        noexcept;

    void sendMessageWithCacheSaving (
        const TransactionMessage::Shared kMessage,
        ContractorID contractorID,
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
        pair<ContractorID, TransactionMessage::Shared>);

    void onConfirmationNotStronglyRequiredMessageReadyToResend(
        BaseAddress::Shared contractorAddress,
        MaxFlowCalculationConfirmationMessage::Shared message);

    void onPingMessageReadyToResend(
        pair<ContractorID, PingMessage::Shared>);

    void onClearTopologyCache(
        const SerializedEquivalent equivalent,
        BaseAddress::Shared nodeAddress);

    static string logHeader()
    noexcept;

    LoggerStream info() const
    noexcept;

    LoggerStream debug() const
    noexcept;

    LoggerStream error() const
    noexcept;

protected:
    IOService &mIOService;
    unique_ptr<CommunicatorStorageHandler> mCommunicatorStorageHandler;
    TailManager &mTailManager;
    Logger &mLog;

    unique_ptr<UDPSocket> mSocket;
    ContractorsManager *mContractorsManager;
    unique_ptr<IncomingMessagesHandler> mIncomingMessagesHandler;
    unique_ptr<OutgoingMessagesHandler> mOutgoingMessagesHandler;
    unique_ptr<ConfirmationRequiredMessagesHandler> mConfirmationRequiredMessagesHandler;
    unique_ptr<ConfirmationNotStronglyRequiredMessagesHandler> mConfirmationNotStronglyRequiredMessagesHandler;
    unique_ptr<ConfirmationResponseMessagesHandler> mConfirmationResponseMessagesHandler;
    unique_ptr<PingMessagesHandler> mPingMessagesHandler;
};


#endif //GEO_NETWORK_CLIENT_COMMUNICATOR_H
