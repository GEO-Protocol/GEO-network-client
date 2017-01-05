#include "PacketHeader.h"

PacketHeader::PacketHeader() {}

PacketHeader::PacketHeader(const uint16_t channel,
                           const uint16_t packetNumber,
                           const uint16_t totalPacketsCount,
                           const uint16_t bytesCount) {

    mChannel = channel;
    mPacketNumber = packetNumber;
    mTotalPacketsCount = totalPacketsCount;
    mBytesCount = bytesCount;
}

PacketHeader::~PacketHeader() {}

const uint16_t PacketHeader::channel() const {
    return mChannel;
}

const uint16_t PacketHeader::packetNumber() const {
    return mPacketNumber;
}

const uint16_t PacketHeader::totalPacketsCount() const {
    return mTotalPacketsCount;
}

const uint16_t PacketHeader::bytesCount() const {
    return mBytesCount;
}
