#include "OutgoingMessagesHandler.h"


OutgoingMessagesHandler::OutgoingMessagesHandler(
    IOService &ioService,
    UDPSocket &socket,
    ContractorsManager *contractorsManager,
    Logger &log)
    noexcept :

    mLog(log),
    mNodes(
        ioService,
        socket,
        contractorsManager,
        log)
{}

void OutgoingMessagesHandler::sendMessage(
    const Message::Shared message,
    const ContractorID addressee)
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

void OutgoingMessagesHandler::sendMessage(
    const Message::Shared message,
    const BaseAddress::Shared address)
{
    try {
        auto node = mNodes.handler(address);
        node->sendMessage(message);

    } catch (exception &e) {
        mLog.error("OutgoingMessagesHandler::sendMessage")
                << "Attempt to send message to the node (" << address->fullAddress() << ") failed with exception. "
                << "Details are: " << e.what() << ". "
                << "Message type: " << message->typeID();
    }
}
