#ifndef GEO_NETWORK_CLIENT_OUTGOINGMESSAGESHANDLER_H
#define GEO_NETWORK_CLIENT_OUTGOINGMESSAGESHANDLER_H

#include "../../common/Types.h"
#include "../../common/memory/MemoryUtils.h"

#include "../messages/Message.hpp"

#include "../channels/packet/PacketHeader.h"
#include "../channels/packet/Packet.h"
#include "../channels/channel/Channel.h"

#include "boost/crc.hpp"

#include <vector>

using namespace std;

class OutgoingMessagesHandler {

public:
    OutgoingMessagesHandler();

public:
    void processOutgoingMessage(
        Message::Shared message,
        uint16_t channelNumber,
        Channel::Shared channel);

private:
    pair<Packet::Shared, uint16_t> makeCRCPacket(
        pair<BytesShared, size_t> messageBytesAndCount,
        uint16_t channelNumber);

    Packet::Shared makePacket(
        byte *buffer,
        size_t bytesCount,
        uint16_t packetNumber,
        uint16_t packetsCount,
        uint16_t channelNumber
    );

private:
    const size_t kMaxPacketBodySize = 490;
    const size_t kCRCDataSize = 4;
    const uint16_t kCRCPacketNumber = 0;

};


#endif //GEO_NETWORK_CLIENT_OUTGOINGMESSAGESHANDLER_H
