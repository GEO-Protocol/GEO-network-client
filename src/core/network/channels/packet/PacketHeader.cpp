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

    size_t headerBytesSize = kHeaderSize;

    BytesShared headerBytes = tryCalloc(
        headerBytesSize);
    size_t headerBytesOffset = 0;
    //----------------------------------------------
    memcpy(
        headerBytes.get(),
        &mPacketBytesCount,
        kHeaderRecordSize);
    headerBytesOffset += kHeaderRecordSize;
    //----------------------------------------------
    memcpy(
        headerBytes.get() + headerBytesOffset,
        &mChannel,
        kHeaderRecordSize);
    headerBytesOffset += kHeaderRecordSize;
    //----------------------------------------------
    memcpy(
        headerBytes.get() + headerBytesOffset,
        &mPacketNumber,
        kHeaderRecordSize);
    headerBytesOffset += kHeaderRecordSize;
    //----------------------------------------------
    memcpy(
        headerBytes.get() + headerBytesOffset,
        &mTotalPacketsCount,
        kHeaderRecordSize);
    //----------------------------------------------
    ConstBytesShared constHeaderBytes = const_pointer_cast<const byte>(headerBytes);

    return make_pair(
        constHeaderBytes,
        headerBytesSize);
}
