#ifndef GEO_NETWORK_CLIENT_INCOMINGCONNECTIONSHANDLER_H
#define GEO_NETWORK_CLIENT_INCOMINGCONNECTIONSHANDLER_H

#include "../../common/Types.h"

#include "../messages/Message.h"

#include "../channels/packet/PacketHeader.h"
#include "../channels/packet/Packet.h"
#include "../channels/channel/Channel.h"
#include "../channels/manager/ChannelsManager.h"

#include "../../common/exceptions/ValueError.h"
#include "../../common/exceptions/ConflictError.h"

#include <boost/date_time.hpp>
#include <boost/asio.hpp>

#include <vector>


using namespace std;
using namespace boost::asio::ip;


class MessagesParser {

public:
    pair<bool, Message::Shared> processMessage(
        const byte *messagePart,
        const size_t receivedBytesCount);

private:
    pair<bool, Message::Shared> tryDeserializeMessage(
        const byte *messagePart);

    pair<bool, Message::Shared> messageInvalidOrIncomplete();

private:
    const size_t kMessageIdentifierSize = 2;
    const size_t kMininalMessageSize = kMessageIdentifierSize + 1;

};


class IncomingMessagesHandler {

public:
    IncomingMessagesHandler();

    ~IncomingMessagesHandler();

    void processIncomingMessage(
        udp::endpoint &clientEndpoint,
        const byte *messagePart,
        const size_t receivedBytesCount);

private:
    void tryCollectPacket();

    void cutPacketFromBuffer(
        size_t bytesCount);

private:
    ChannelsManager *mChannelsManager;
    MessagesParser *mMessagesParser;

    vector<byte> mPacketsBuffer;
};

#endif //GEO_NETWORK_CLIENT_INCOMINGCONNECTIONSHANDLER_H
