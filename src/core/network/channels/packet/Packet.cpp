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
}

Packet::~Packet() {}

PacketHeader::SharedConst Packet::header() const {

    return mPacketHeader;
}

ConstBytesShared Packet::body() const {

    return mBytes;
}
