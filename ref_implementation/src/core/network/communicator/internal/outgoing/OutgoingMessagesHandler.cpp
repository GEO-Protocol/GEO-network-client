/**
 * This file is part of GEO Project.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Project/GEO-Project/blob/master/LICENSE.md
 *
 * No part of GEO Project, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

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
