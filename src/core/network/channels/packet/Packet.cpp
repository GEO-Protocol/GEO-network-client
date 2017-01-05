#include "Packet.h"

Packet::Packet(
    PacketHeader *packetHeader,
    const byte *bytes) :

    mPacketHeader(PacketHeader::Shared(packetHeader)),
    mBytes(BytesShared(bytes, free)){}

Packet::~Packet() {}

PacketHeader::SharedConst Packet::header() const {

    return mPacketHeader;
}

ConstBytesShared Packet::body() const {

    return mBytes;
}
