#ifndef GEO_NETWORK_CLIENT_INCOMINGCONNECTIONSHANDLER_H
#define GEO_NETWORK_CLIENT_INCOMINGCONNECTIONSHANDLER_H

#include "../../common/Types.h"

#include "../channels/packet/PacketHeader.h"
#include "../channels/packet/Packet.h"
#include "../channels/channel/Channel.h"
#include "../channels/manager/ChannelsManager.h"

#include "../messages/Message.h"
#include "../messages/incoming/trust_lines/AcceptTrustLineMessage.h"
#include "../messages/incoming/trust_lines/RejectTrustLineMessage.h"
#include "../messages/incoming/trust_lines/UpdateTrustLineMessage.h"
#include "../messages/response/Response.h"

#include "../../common/exceptions/ValueError.h"
#include "../../common/exceptions/ConflictError.h"

#include <boost/date_time.hpp>
#include <boost/asio.hpp>
#include <boost/signals2.hpp>

#include <vector>


using namespace std;
using namespace boost::asio::ip;

namespace signals = boost::signals2;

class MessagesParser {

public:
    pair<bool, Message::Shared> processMessage(
        ConstBytesShared messagePart,
        const size_t receivedBytesCount);

private:
    pair<bool, Message::Shared> tryDeserializeMessage(
        ConstBytesShared messagePart);

    pair<bool, Message::Shared> tryDeserializeRequest(
        const uint16_t messageIdentifier,
        const byte *messagePart);

    pair<bool, Message::Shared> tryDeserializeResponse(
        const uint16_t messageIdentifier,
        const byte *messagePart);

    pair<bool, Message::Shared> messageInvalidOrIncomplete();

private:
    const size_t kMessageIdentifierSize = 2;
    const size_t kMininalMessageSize = kMessageIdentifierSize + 1;

};


class IncomingMessagesHandler {
public:
    signals::signal<void(Message::Shared)> messageParsedSignal;

public:
    IncomingMessagesHandler(
        ChannelsManager *channelsManager);

    ~IncomingMessagesHandler();

    void processIncomingMessage(
        udp::endpoint &clientEndpoint,
        const byte *messagePart,
        const size_t receivedBytesCount);

private:
    void tryCollectPacket(
        udp::endpoint &clientEndpoint);

    void cutPacketFromBuffer(
        size_t bytesCount);

private:
    ChannelsManager *mChannelsManager;

    MessagesParser *mMessagesParser;

    vector<byte> mPacketsBuffer;
};

#endif //GEO_NETWORK_CLIENT_INCOMINGCONNECTIONSHANDLER_H
