#include "OutgoingMessagesHandler.h"

OutgoingMessagesHandler::OutgoingMessagesHandler() {}

void OutgoingMessagesHandler::processOutgoingMessage(
    Message::Shared message,
    uint16_t channelNumber,
    Channel::Shared channel) {

    auto messageBytesAndCount = message->serializeToBytes();

    auto crcPacketAndCount = makeCRCPacket(
        messageBytesAndCount,
        channelNumber
    );

    channel->setOutgoingPacketsCount(crcPacketAndCount.second);

    channel->addPacket(
        Channel::kCRCPacketNumber(),
        crcPacketAndCount.first
    );

    byte *serializedMessageBuffer = messageBytesAndCount.first.get();
    uint16_t dataPacketsCount = (uint16_t) (crcPacketAndCount.second - 1);

    for (uint16_t packetNumber = 1; packetNumber <= dataPacketsCount; ++ packetNumber) {
        size_t packetSize;
        size_t offset = 0;

        if (dataPacketsCount > 1) {
            if (packetNumber == dataPacketsCount) {
                packetSize = messageBytesAndCount.second - kMaxPacketBodySize * packetNumber;

            } else {
                packetSize = kMaxPacketBodySize;

            }

        } else {
            packetSize = messageBytesAndCount.second;
        }

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
    uint32_t controlSum = (uint32_t) crc.checksum();
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
        (const uint16_t) (PacketHeader::kHeaderSize + kCRCDataSize)
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
      (const uint16_t) (PacketHeader::kHeaderSize + bytesCount)
    );

    Packet *packet = new Packet(
      packetHeader,
      buffer,
      packetHeader->bodyBytesCount()
    );

    return Packet::Shared(packet);
}

