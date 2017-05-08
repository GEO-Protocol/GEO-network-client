#include "Communicator.h"


Communicator::Communicator(
    IOService &ioService,
    const string &interface,
    const uint16_t port,
    const string &uuid2AddressHost,
    const uint16_t uuid2AddressPort,
    Logger &logger):

    mIOService(ioService),
    mInterface(interface),
    mPort(port),
    mSocket(
        make_unique<UDPSocket>(
            ioService,
            boost::asio::ip::udp::endpoint(
                boost::asio::ip::udp::v4(),
                port))),
    mUUID2AddressService(
        make_unique<UUID2Address>(
            ioService,
            uuid2AddressHost,
            uuid2AddressPort)),
    mIncomingMessagesHandler(
        make_unique<IncomingMessagesHandler>(
            ioService,
            *mSocket,
            logger)),
    mOutgoingMessagesHandler(
        make_unique<OutgoingMessagesHandler>(
            ioService,
            *mSocket,
            *mUUID2AddressService,
            logger)),
    mLog(logger)
{
    // Chain signals directly.
    // If mIncomingMessagesHandler would signal about newly parsed message -
    // it would be directly processed by the internal core logic,
    // with no intermediate slot in the communicator.
    mIncomingMessagesHandler->signalMessageParsed.connect(
        signalMessageReceived);
}

/**
 * Registers current node into the UUID2Address service.
 *
 * @returns true in case of success, otherwise - returns false.
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
            << "Can't register in global nodes addresses cache. "
            << "Internal error: " << e.what();
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
