#include "Communicator.h"


Communicator::Communicator(
    IOService &IOService,
    const Host &interface,
    const Port port,
    const Host &UUID2AddressHost,
    const Port UUID2AddressPort,
    Logger &logger):

    mInterface(interface),
    mPort(port),
    mIOService(IOService),
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
            logger))
{
    // Direct signals chaining.
    mIncomingMessagesHandler->signalMessageParsed.connect(
        signalMessageReceived);
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
        mLog.error("Communicator::joinUUID2Address")
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
    mOutgoingMessagesHandler->sendMessage(
        message,
        contractorUUID);
}
