/**
 * This file is part of GEO Project.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Project/GEO-Project/blob/master/LICENSE.md
 *
 * No part of GEO Project, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

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
 * In case if some internal errors would occure - all of them would be only logged.
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
    mConfirmationRequiredMessagesHandler->tryEnqueueMessage(
        contractorUUID,
        message);

    mOutgoingMessagesHandler->sendMessage(
        message,
        contractorUUID);
}

void Communicator::processConfirmationMessage(
    const NodeUUID &contractorUUID,
    ConfirmationMessage::Shared confirmationMessage)
{
    mConfirmationRequiredMessagesHandler->tryProcessConfirmation(
        contractorUUID,
        confirmationMessage);
}

void Communicator::onMessageReceived(
    Message::Shared message)
{
    // these messages are inherited from DestinationMessage
    // and should be checked if they were delivered on address
    if (message->typeID() >= 100 && message->typeID() <= 102) {
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

    // In case if received message is of type "confirmation message" -
    // then it must not be transferred for further processing.
    // Instead of that, it must be transferred for processing into
    // confirmation required messages handler.
    if (message->typeID() == Message::System_Confirmation) {
        const auto kConfirmationMessage =
            static_pointer_cast<ConfirmationMessage>(message);
        if (kConfirmationMessage->state() != ConfirmationMessage::ContractorBanned) {
            mConfirmationRequiredMessagesHandler->tryProcessConfirmation(
                kConfirmationMessage->senderUUID,
                kConfirmationMessage);
            return;
        }
    }

    signalMessageReceived(message);
}

void Communicator::onConfirmationRequiredMessageReadyToResend(
    pair<NodeUUID, TransactionMessage::Shared> adreseeAndMessage)
{
    mOutgoingMessagesHandler->sendMessage(
        static_pointer_cast<Message>(adreseeAndMessage.second),
        adreseeAndMessage.first);
}

string Communicator::logHeader()
noexcept
{
    return "[Communicator]";
}

LoggerStream Communicator::error() const
noexcept
{
    return mLog.error(logHeader());
}
