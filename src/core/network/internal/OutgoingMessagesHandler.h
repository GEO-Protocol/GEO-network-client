#ifndef GEO_NETWORK_CLIENT_OUTGOINGMESSAGESHANDLER_H
#define GEO_NETWORK_CLIENT_OUTGOINGMESSAGESHANDLER_H

#include "../messages/Message.h"

#include "../channels/packet/PacketHeader.h"
#include "../channels/packet/Packet.h"

#include "boost/crc.hpp"

#include <vector>

using namespace std;

class OutgoingMessagesHandler {

public:
    OutgoingMessagesHandler();

public:
    vector<Packet::SharedConst>* processOutgoingMessage(
        Message::Shared message,
        uint16_t channelNumber);

private:
    pair<Packet::SharedConst, uint16_t> makeCRCPacket(
        pair<ConstBytesShared, size_t> serialiedMessage,
        uint16_t channelNumber);

    Packet::SharedConst makePacket(
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
