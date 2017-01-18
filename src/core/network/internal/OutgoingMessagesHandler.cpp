#include "OutgoingMessagesHandler.h"

OutgoingMessagesHandler::OutgoingMessagesHandler() {}

vector<Packet::SharedConst>* OutgoingMessagesHandler::processOutgoingMessage(
    Message::Shared message,
    uint16_t channelNumber) {

    // Message's serialized data and data size
    auto serializedMessage = message->serialize();

    // Container for packets allocated in memory
    vector<Packet::SharedConst> *packets = new vector<Packet::SharedConst>();

    // Create crc packet and calculate packet's count
    auto crcPacket = makeCRCPacket(
        serializedMessage,
        channelNumber
    );
    // Push back crc packet in container
    packets->push_back(crcPacket.first);

    // Create other packages
    byte *offsetSerializedBuffer = const_cast<byte *> (serializedMessage.first.get());
    uint16_t dataPacketsCount = crcPacket.second - 1; // crc packet excluded

    for (uint16_t packetNumber = 1; packetNumber <= dataPacketsCount; ++ packetNumber) {
        size_t packetSize;
        size_t offset = 0;

        // Calculate size of next packet.
        // If total data packets count greater than 1 -
        // size of next packet will be calculate by nested 'if' statement.
        // Else if have only one packet - packet size will be equals to size of message's data.
        if (dataPacketsCount > 1) {
            // If iteration on last packet -
            // size of next packet will be equals serialized message data - maximal packet's body size * packet sequence number.
            // Else packet size will be equals to maximal packet's body size.
            if (packetNumber == dataPacketsCount) {
                packetSize = serializedMessage.second - kMaxPacketBodySize * packetNumber;

            } else {
                packetSize = kMaxPacketBodySize;

            }

        } else {
            packetSize = serializedMessage.second;
        }

        // Calculate offset to next part of serialized data.
        if (packetNumber == 1) {
            offset = 0;

        } else {
            offset += kMaxPacketBodySize;
        }

        auto packet = makePacket(
            offsetSerializedBuffer + offset,
            packetSize,
            packetNumber,
            crcPacket.second,
            channelNumber
        );
        packets->push_back(packet);
    }

    return packets;
}

pair<Packet::SharedConst, uint16_t> OutgoingMessagesHandler::makeCRCPacket(
    pair<ConstBytesShared, size_t> serialiedMessage,
    uint16_t channelNumber) {

    uint16_t packetsCount;
    if (kMaxPacketBodySize < serialiedMessage.second) {
        packetsCount = (uint16_t) (serialiedMessage.second / kMaxPacketBodySize + 1); // integer packets count + crc packet;
        if (packetsCount == 2) { // if calculated only one integer data packet - add one more packet, it's rest data
            packetsCount += 1;
        }

    } else {
        packetsCount = 2; // data packet + crc packet
    }

    boost::crc_32_type crc;
    crc.process_bytes(
        serialiedMessage.first.get(),
        serialiedMessage.second
    );
    uint32_t controlSum = crc.checksum();
    byte *controlSumBytes = (byte *) malloc(kCRCDataSize);
    memset(
        controlSumBytes,
        0,
        kCRCDataSize
    );
    memcpy(
        controlSumBytes,
        &controlSum,
        kCRCDataSize
    );

    PacketHeader *packetHeader = new PacketHeader(
        channelNumber,
        kCRCPacketNumber,
        packetsCount,
        PacketHeader::kHeaderSize + kCRCDataSize
    );

    Packet *packet = new Packet(
        packetHeader,
        controlSumBytes,
        packetHeader->bodyBytesCount()
    );

    return make_pair(
        Packet::SharedConst(packet),
        packetsCount
    );
}


Packet::SharedConst OutgoingMessagesHandler::makePacket(
    byte *buffer,
    size_t bytesCount,
    uint16_t packetNumber,
    uint16_t packetsCount,
    uint16_t channelNumber) {

    PacketHeader *packetHeader = new PacketHeader(
      channelNumber,
      packetNumber,
      packetsCount,
      PacketHeader::kHeaderSize + bytesCount
    );

    Packet *packet = new Packet(
      packetHeader,
      buffer,
      packetHeader->bodyBytesCount()
    );

    return Packet::SharedConst(packet);
}

