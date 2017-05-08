#include "OutgoingMessagesHandler.h"


OutgoingMessagesHandler::OutgoingMessagesHandler(
    IOService &ioService,
    UDPSocket &socket,
    UUID2Address &uuid2AddressService,
    Logger &log)
    noexcept :

    mLog(log),
    mNodes(
        ioService,
        socket,
        uuid2AddressService,
        log)
{}

void OutgoingMessagesHandler::sendMessage(
    const Message::Shared message,
    const NodeUUID &addressee)
{
    try {
        auto node = mNodes.handler(addressee);
        node->sendMessage(message);

    } catch (exception &e) {
        mLog.error("OutgoingMessagesHandler::sendMessage")
            << "Attempt to send message to the node (" << addressee << ") failed with exception. "
            << "Details are: " << e.what() << ". "
            << "Message type: " << message->typeID();
    }
}
