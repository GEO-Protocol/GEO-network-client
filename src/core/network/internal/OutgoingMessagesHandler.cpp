#include "OutgoingMessagesHandler.h"

OutgoingMessagesHandler::OutgoingMessagesHandler() {}

void OutgoingMessagesHandler::processOutgoingMessage(
    Message::Shared message,
    uint16_t channelNumber,
    Channel::Shared channel) {

    // Message's serialized data and data size
    auto messageBytesAndCount = message->serialize();


    // Create crc packet and calculate packet's count
    auto crcPacketAndCount = makeCRCPacket(
        messageBytesAndCount,
        channelNumber
    );

    // Remember packets count to be sended
    channel->setOutgoingPacketsCount(crcPacketAndCount.second);

    // Push crc packet in incomingChannel
    channel->addPacket(
        Channel::kCRCPacketNumber(),
        crcPacketAndCount.first
    );

    // Create other packages
    byte *serializedMessageBuffer = const_cast<byte *> (messageBytesAndCount.first.get());
    uint16_t dataPacketsCount = crcPacketAndCount.second - 1; // crc packet excluded

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
                packetSize = messageBytesAndCount.second - kMaxPacketBodySize * packetNumber;

            } else {
                packetSize = kMaxPacketBodySize;

            }

        } else {
            packetSize = messageBytesAndCount.second;
        }

        // Calculate offset to next part of serialized data.
        if (packetNumber == 1) {
            offset = 0;

        } else {
            offset += kMaxPacketBodySize;
        }

        auto packet = makePacket(
            serializedMessageBuffer + offset,
            packetSize,
            packetNumber,
            crcPacketAndCount.second,
            channelNumber
        );

        channel->addPacket(
            packetNumber,
            packet
        );

    }
}

pair<Packet::Shared, uint16_t> OutgoingMessagesHandler::makeCRCPacket(
    pair<ConstBytesShared, size_t> messageBytesAndCount,
    uint16_t channelNumber) {

    uint16_t packetsCount;
    if (kMaxPacketBodySize < messageBytesAndCount.second) {
        packetsCount = (uint16_t) (messageBytesAndCount.second / kMaxPacketBodySize + 1); // integer packets count + crc packet;
        if (packetsCount == 2) { // if calculated only one integer data packet - add one more packet, it's rest data
            packetsCount += 1;
        }

    } else {
        packetsCount = 2; // data packet + crc packet
    }

    boost::crc_32_type crc;
    crc.process_bytes(
        messageBytesAndCount.first.get(),
        messageBytesAndCount.second
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
        Packet::Shared(packet),
        packetsCount
    );
}


Packet::Shared OutgoingMessagesHandler::makePacket(
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

    return Packet::Shared(packet);
}

