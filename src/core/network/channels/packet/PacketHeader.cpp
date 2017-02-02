#include "PacketHeader.h"

PacketHeader::PacketHeader() {}

PacketHeader::PacketHeader(const uint16_t channel,
                           const uint16_t packetNumber,
                           const uint16_t totalPacketsCount,
                           const uint16_t bytesCount) {

    mChannel = channel;
    mPacketNumber = packetNumber;
    mTotalPacketsCount = totalPacketsCount;
    mPacketBytesCount = bytesCount;
    mBodyBytesCount = mPacketBytesCount - (uint16_t)kHeaderSize;
}

PacketHeader::~PacketHeader() {}

const uint16_t PacketHeader::channelNumber() const {

    return mChannel;
}

const uint16_t PacketHeader::packetNumber() const {

    return mPacketNumber;
}

const uint16_t PacketHeader::totalPacketsCount() const {

    return mTotalPacketsCount;
}

const uint16_t PacketHeader::packetBytesCount() const {

    return mPacketBytesCount;
}

const uint16_t PacketHeader::bodyBytesCount() const {

    return mBodyBytesCount;
}

pair<ConstBytesShared, size_t> PacketHeader::bytes() const {

    byte * headerBytes = (byte *) calloc (kHeaderSize, sizeof(byte));
    memcpy(
        headerBytes,
        &mPacketBytesCount,
        kHeaderRecordSize
    );

    memcpy(
        headerBytes + kHeaderRecordSize,
        &mChannel,
        kHeaderRecordSize
    );

    memcpy(
        headerBytes + kHeaderRecordSize * 2,
        &mPacketNumber,
        kHeaderRecordSize
    );

    memcpy(
        headerBytes + kHeaderRecordSize * 3,
        &mTotalPacketsCount,
        kHeaderRecordSize
    );

    return make_pair(
        ConstBytesShared(headerBytes, free),
        8);
}
