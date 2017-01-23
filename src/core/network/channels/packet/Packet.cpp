#include "Packet.h"

Packet::Packet(
    PacketHeader *packetHeader,
    byte *bytes,
    size_t bytesCount) :

    mPacketHeader(PacketHeader::Shared(packetHeader)){

    byte *data = (byte *) malloc(bytesCount);
    memcpy(
        data,
        bytes,
        bytesCount
    );

    mBytes = BytesShared(data, free);
}

Packet::~Packet() {}

PacketHeader::SharedConst Packet::header() const {

    return mPacketHeader;
}

ConstBytesShared Packet::body() const {

    return mBytes;
}

vector<byte> Packet::packetBytes() const {

    byte* packet = (byte *) malloc(mPacketHeader->packetBytesCount());
    memcpy(
      packet,
      mPacketHeader->bytes().first.get(),
      PacketHeader::kHeaderSize
    );

    memcpy(
        packet + PacketHeader::kHeaderSize,
        mBytes.get(),
        mPacketHeader->bodyBytesCount()
    );

    vector<byte> bytes;
    for (size_t i = 0; i < mPacketHeader->packetBytesCount(); ++i) {
        bytes.push_back(packet[i]);
    }
    delete packet;

    return bytes;
}
