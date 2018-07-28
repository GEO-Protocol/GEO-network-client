#include "Communicator.h"


Communicator::Communicator(
    IOService &IOService,
    const Host &interface,
    const Port port,
    const Host &UUID2AddressHost,
    const Port UUID2AddressPort,
    const NodeUUID &nodeUUID,
    Logger &logger):

    mInterface(interface),
    mPort(port),
    mIOService(IOService),
    mNodeUUID(nodeUUID),
    mLog(logger),
    mSocket(
        make_unique<UDPSocket>(
            IOService,
            udp::endpoint(
                udp::v4(),
                port))),

    mUUID2AddressService(
        make_unique<UUID2Address>(
            IOService,
            UUID2AddressHost,
            UUID2AddressPort)),

    mIncomingMessagesHandler(
        make_unique<IncomingMessagesHandler>(
            IOService,
            *mSocket,
            logger)),

    mOutgoingMessagesHandler(
        make_unique<OutgoingMessagesHandler>(
            IOService,
            *mSocket,
            *mUUID2AddressService,
            logger)),

    mCommunicatorStorageHandler(
        make_unique<CommunicatorStorageHandler>(
            // todo : move this consts to Core.h
            "io",
            "communicatorStorageDB",
            logger)),

    mConfirmationRequiredMessagesHandler(
        make_unique<ConfirmationRequiredMessagesHandler>(
            IOService,
            mCommunicatorStorageHandler.get(),
            logger)),

    mConfirmationNotStronglyRequiredMessagesHandler(
        make_unique<ConfirmationNotStronglyRequiredMessagesHandler>(
            IOService,
            logger)),

    mConfirmationResponseMessagesHandler(
        make_unique<ConfirmationResponseMessagesHandler>(
            logger))
{
    // Direct signals chaining.
    mIncomingMessagesHandler->signalMessageParsed.connect(
        boost::bind(
            &Communicator::onMessageReceived,
            this,
            _1));

    mConfirmationRequiredMessagesHandler->signalOutgoingMessageReady.connect(
        boost::bind(
            &Communicator::onConfirmationRequiredMessageReadyToResend,
            this,
            _1));

    mConfirmationNotStronglyRequiredMessagesHandler->signalOutgoingMessageReady.connect(
        boost::bind(
            &Communicator::onConfirmationNotStronglyRequiredMessageReadyToResend,
            this,
            _1));

    mConfirmationNotStronglyRequiredMessagesHandler->signalClearTopologyCache.connect(
        boost::bind(
            &Communicator::onClearTopologyCache,
            this,
            _1,
            _2));
}

/**
 * Registers current node into the UUID2Address service.
 *
 * @returns "true" in case of success, otherwise - returns "false".
 */
bool Communicator::joinUUID2Address(
    const NodeUUID &nodeUUID)
    noexcept
{
    try {
        mUUID2AddressService->registerInGlobalCache(
            nodeUUID,
            mInterface,
            mPort);

        return true;

    } catch (std::exception &e) {
        error() << "joinUUID2Address: "
            << "Can't register in global nodes addresses space. "
            << "Internal error details: " << e.what();
    }

    return false;
}

void Communicator::beginAcceptMessages()
    noexcept
{
    mIncomingMessagesHandler->beginReceivingData();
}

/**
 * Tries to send message to the remote node.
 * Because there is no way to know, if the message was delivered - this method doesn't returns anything.
 * Internal transactions logic must take care about the case when message was lost.
 *
 * This method also doesn't reports any internal errors.
 * In case if some internal errors would occur - all of them would be only logged.
 *
 *
 * @param message - message that must be sent to the remote node.
 * @param contractorUUID - uuid of the remote node which should receive the message.
 */
void Communicator::sendMessage (
    const Message::Shared message,
    const NodeUUID &contractorUUID)
    noexcept
{
    // Filter outgoing messages for confirmation-required messages.
    if (message->isAddToConfirmationNotStronglyRequiredMessagesHandler()) {
        mConfirmationNotStronglyRequiredMessagesHandler->tryEnqueueMessage(
            contractorUUID,
            message);
    }

    else if (message->isAddToConfirmationRequiredMessagesHandler()) {
        mConfirmationRequiredMessagesHandler->tryEnqueueMessage(
            contractorUUID,
            message);
    }

    mOutgoingMessagesHandler->sendMessage(
        message,
        contractorUUID);
}

void Communicator::sendMessageWithCacheSaving(
    const TransactionMessage::Shared message,
    const NodeUUID &contractorUUID,
    Message::MessageType incomingMessageTypeFilter)
    noexcept
{
    mConfirmationResponseMessagesHandler->addCachedMessage(
        contractorUUID,
        message,
        incomingMessageTypeFilter);

    mOutgoingMessagesHandler->sendMessage(
        message,
        contractorUUID);
}

void Communicator::processConfirmationMessage(
    ConfirmationMessage::Shared confirmationMessage)
{
    mConfirmationRequiredMessagesHandler->tryProcessConfirmation(
        confirmationMessage);
}

void Communicator::onMessageReceived(
    Message::Shared message)
{
    // these messages contain parts of topology and after theirs receiving we should send confirmation
    if (message->typeID() == Message::MaxFlow_ResultMaxFlowCalculation ||
            message->typeID() == Message::MaxFlow_ResultMaxFlowCalculationFromGateway) {
        const auto kResultMaxFlowCalculationMessage =
                static_pointer_cast<MaxFlowCalculationConfirmationMessage>(message);
        sendMessage(
            make_shared<MaxFlowCalculationConfirmationMessage>(
                kResultMaxFlowCalculationMessage->equivalent(),
                mNodeUUID,
                kResultMaxFlowCalculationMessage->confirmationID()),
            kResultMaxFlowCalculationMessage->senderUUID);
    }

    // In case if received message is of type "max flow confirmation message" -
    // then it must not be transferred for further processing.
    // Instead of that, it must be transferred for processing into
    // confirmation not strongly required messages handler.
    else if (message->typeID() == Message::MaxFlow_Confirmation) {
        const auto kConfirmationMessage =
                static_pointer_cast<MaxFlowCalculationConfirmationMessage>(message);
            mConfirmationNotStronglyRequiredMessagesHandler->tryProcessConfirmation(
                kConfirmationMessage);
            return;
    }

    // these messages are inherited from DestinationMessage
    // and should be checked if they were delivered on address
    else if (message->isDestinationMessage()) {
        const auto kDestinationMessage =
            static_pointer_cast<DestinationMessage>(message);
        if (kDestinationMessage->destinationUUID() != mNodeUUID) {
            error() << "onMessageReceived: "
                    << "Invalid destinationUUID. "
                    << "Message type: " << kDestinationMessage->typeID()
                    << ", destination UUID: " << kDestinationMessage->destinationUUID()
                    << ", sender UUID: " << kDestinationMessage->senderUUID;
            return;
        }
    }

    if (message->isCheckCachedResponse()) {
        auto incomingTransactionMessage = static_pointer_cast<TransactionMessage>(message);
        auto cachedResponse = mConfirmationResponseMessagesHandler->getCachedMessage(incomingTransactionMessage);
        if (cachedResponse != nullptr) {
            sendMessage(
                cachedResponse,
                incomingTransactionMessage->senderUUID);
#ifdef DEBUG_LOG_NETWORK_COMMUNICATOR
            debug() << "send cached response";
#endif
            return;
        }
    }

    // In case if received message is of type "confirmation message" -
    // then it must not be transferred for further processing.
    // Instead of that, it must be transferred for processing into
    // confirmation required messages handler.
    if (message->typeID() == Message::System_Confirmation) {
        mConfirmationRequiredMessagesHandler->tryProcessConfirmation(
            static_pointer_cast<ConfirmationMessage>(message));
        return;

    } else if (message->typeID() == Message::RoutingTableResponse) {
        const auto kConfirmationMessage =
            static_pointer_cast<ConfirmationMessage>(message);
        mConfirmationRequiredMessagesHandler->tryProcessConfirmation(
            kConfirmationMessage);
    }

    signalMessageReceived(message);
}

void Communicator::onConfirmationRequiredMessageReadyToResend(
    pair<NodeUUID, TransactionMessage::Shared> addresseeAndMessage)
{
    mOutgoingMessagesHandler->sendMessage(
        static_pointer_cast<Message>(
            addresseeAndMessage.second),
        addresseeAndMessage.first);
}

void Communicator::onConfirmationNotStronglyRequiredMessageReadyToResend(
    pair<NodeUUID, MaxFlowCalculationConfirmationMessage::Shared> addresseeAndMessage)
{
    mOutgoingMessagesHandler->sendMessage(
        static_pointer_cast<Message>(
            addresseeAndMessage.second),
        addresseeAndMessage.first);
}

void Communicator::onClearTopologyCache(
    const SerializedEquivalent equivalent,
    const NodeUUID &nodeUUID)
{
    signalClearTopologyCache(
        equivalent,
        nodeUUID);
}

string Communicator::logHeader()
noexcept
{
    return "[Communicator]";
}

LoggerStream Communicator::info() const
noexcept
{
    return mLog.info(logHeader());
}

LoggerStream Communicator::debug() const
noexcept
{
    return mLog.debug(logHeader());
}

LoggerStream Communicator::error() const
noexcept
{
    return mLog.error(logHeader());
}
