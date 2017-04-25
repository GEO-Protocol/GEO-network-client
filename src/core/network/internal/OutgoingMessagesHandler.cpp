#include "OutgoingMessagesHandler.h"

OutgoingMessagesHandler::OutgoingMessagesHandler() {}

void OutgoingMessagesHandler::processOutgoingMessage(
    Message::Shared message,
    uint16_t channelNumber,
    Channel::Shared channel) {

    auto messageBytesAndCount = message->serializeToBytes();

    auto crcPacketAndCount = makeCRCPacket(
        messageBytesAndCount,
        channelNumber);

//    cout << "OutgoingMessagesHandler\tchannel: " << channelNumber << "\tpacket: "
//         << crcPacketAndCount.first->header()->packetNumber() << "\ttotal packages: " << totalCountPackages++ << endl;
    channel->setOutgoingPacketsCount(
        crcPacketAndCount.second);

    channel->addPacket(
        Channel::kCRCPacketNumber(),
        crcPacketAndCount.first);

    byte *serializedMessageBuffer = messageBytesAndCount.first.get();
    uint16_t dataPacketsCount = (uint16_t) (crcPacketAndCount.second - 1);

    if (message->isMaxFlowCalculationResponseMessage()) {
        mMaxFlowCalculationTraffic += messageBytesAndCount.second + crcPacketAndCount.second;
    }

    for (uint16_t packetNumber = 1; packetNumber <= dataPacketsCount; ++ packetNumber) {
        size_t packetSize;
        size_t offset = 0;

        if (dataPacketsCount > 1) {
            if (packetNumber == dataPacketsCount) {
                packetSize = messageBytesAndCount.second - (kMaxPacketBodySize * (packetNumber - 1));

            } else {
                packetSize = kMaxPacketBodySize;

            }

        } else {
            packetSize = messageBytesAndCount.second;
        }

        if (packetNumber == 1) {
            offset = 0;

        } else {
            offset = kMaxPacketBodySize * (packetNumber - 1);
        }

        auto packet = makePacket(
            serializedMessageBuffer + offset,
            packetSize,
            packetNumber,
            crcPacketAndCount.second,
            channelNumber);

//        cout << "OutgoingMessagesHandler\tchannel: " << channelNumber << "\tpacket: " << packetNumber
//             << "\ttotal packages: " << totalCountPackages++ << endl;


        channel->addPacket(
            packetNumber,
            packet);

    }
}

pair<Packet::Shared, uint16_t> OutgoingMessagesHandler::makeCRCPacket(
    pair<BytesShared, size_t> messageBytesAndCount,
    uint16_t channelNumber) {

    uint16_t packetsCount;
    if (kMaxPacketBodySize < messageBytesAndCount.second) {
        size_t remain = messageBytesAndCount.second % kMaxPacketBodySize;
        packetsCount = (uint16_t) (messageBytesAndCount.second / kMaxPacketBodySize + 1);

        if (remain > 0) {
            packetsCount += 1;
        }

        if (packetsCount == 2) {
            packetsCount += 1;
        }

    } else {
        packetsCount = 2;
    }

    boost::crc_32_type crc;
    crc.process_bytes(
        messageBytesAndCount.first.get(),
        messageBytesAndCount.second);
    uint32_t controlSum = (uint32_t) crc.checksum();

    BytesShared controlSumBytesShared = tryCalloc(
        kCRCDataSize);
    memcpy(
        controlSumBytesShared.get(),
        &controlSum,
        kCRCDataSize);

    PacketHeader::Shared packetHeaderShared(
        new PacketHeader(
            channelNumber,
            kCRCPacketNumber,
            packetsCount,
            (uint16_t) (PacketHeader::kHeaderSize + kCRCDataSize)));

    Packet::Shared packetShared(
        new Packet(
            packetHeaderShared,
            static_pointer_cast<const byte>(controlSumBytesShared)));

    return make_pair(
        packetShared,
        packetsCount);
}


Packet::Shared OutgoingMessagesHandler::makePacket(
    byte *buffer,
    size_t bytesCount,
    uint16_t packetNumber,
    uint16_t packetsCount,
    uint16_t channelNumber) {

    PacketHeader::Shared packetHeaderShared(
        new PacketHeader(
            channelNumber,
            packetNumber,
            packetsCount,
            (uint16_t)(PacketHeader::kHeaderSize + bytesCount)));

    BytesShared bodyBytesShared = tryCalloc(packetHeaderShared->bodyBytesCount());
    memcpy(
        bodyBytesShared.get(),
        buffer,
        packetHeaderShared->bodyBytesCount());

    Packet::Shared packetShared(
        new Packet(
            packetHeaderShared,
            static_pointer_cast<const byte>(bodyBytesShared)));

    return packetShared;
}


