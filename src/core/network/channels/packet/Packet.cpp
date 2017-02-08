#include "Packet.h"

Packet::Packet(
    PacketHeader::Shared packetHeader,
    ConstBytesShared bytes) :

    mPacketHeader(packetHeader),
    mBytes(bytes){
}

Packet::~Packet() {}

PacketHeader::SharedConst Packet::header() const {

    return mPacketHeader;
}

ConstBytesShared Packet::body() const {

    return mBytes;
}

vector<byte> Packet::packetBytes() const {

    BytesShared packetBytes = tryCalloc(mPacketHeader->packetBytesCount());
    size_t packetBytesOffset = 0;
    //----------------------------------------------
    memcpy(
        packetBytes.get(),
        const_cast<byte *> (mPacketHeader->bytes().first.get()),
        PacketHeader::kHeaderSize
    );
    packetBytesOffset += PacketHeader::kHeaderSize;
    //----------------------------------------------
    memcpy(
        packetBytes.get() + packetBytesOffset,
        mBytes.get(),
        mPacketHeader->bodyBytesCount()
    );
    //----------------------------------------------
    vector<byte> bytes;
    for (size_t i = 0; i < mPacketHeader->packetBytesCount(); ++i) {
        bytes.push_back(packetBytes.get()[i]);
    }

    return bytes;
}
