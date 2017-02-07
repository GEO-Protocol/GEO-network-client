#include "Channel.h"

Channel::Channel() {

    try {
        mPackets = new map<uint16_t, Packet::Shared, less<uint16_t>>();

    } catch (std::bad_alloc&) {
        throw MemoryError("Channel::Channel: "
                              "Can not allocate memory for packets container.");
    }
    rememberCreationTime();
    mExpectedPacketsCount, mOutgoingPacketsCount, mSendedPacketsCount = 0;
}

Channel::~Channel() {

    mPackets->clear();
    delete mPackets;
}

void Channel::addPacket(
    uint16_t position,
    Packet::Shared packet) {

    if (mPackets->count(position) != 0) {
        throw ConflictError("Channel::addPacket: "
                         "Packet with same position is already exist.");
    }

    mExpectedPacketsCount = packet->header()->totalPacketsCount();
    mPackets->insert(
        make_pair(
            position,
            packet
        )
    );

}

bool Channel::checkConsistency() {

    if (mPackets->size() == mExpectedPacketsCount) {
        if (mPackets->count(kCRCPacketNumber()) != 0) {
            uint32_t *controlSum = new(const_cast<byte *> (mPackets->at(kCRCPacketNumber())->body().get())) uint32_t;

            auto channelBytesAndCount = data();
            boost::crc_32_type control;
            control.process_bytes(
                channelBytesAndCount.first.get(),
                channelBytesAndCount.second
            );

            return control.checksum() == *controlSum;
        }
    }
    return false;
}

pair<BytesShared, size_t> Channel::data() {

    size_t totalBytesCount = 0;
    for (auto &numberAndChannel : *mPackets) {
        if (numberAndChannel.first != kCRCPacketNumber()) {
            totalBytesCount += (size_t) numberAndChannel.second->header()->bodyBytesCount();
        }
    }

    byte *data = (byte *) calloc(
        totalBytesCount,
        sizeof(byte)
    );

    size_t nextPacketBytesOffset = 0;
    for (auto &numberAndChannel : *mPackets) {
        if (numberAndChannel.first != kCRCPacketNumber()) {
            memcpy(
                data + nextPacketBytesOffset,
                const_cast<byte *>(numberAndChannel.second->body().get()),
                (size_t) numberAndChannel.second->header()->bodyBytesCount()
            );
            nextPacketBytesOffset += numberAndChannel.second->header()->bodyBytesCount();
        }
    }

    return make_pair(
        BytesShared(
            data,
            free
        ),
        totalBytesCount
    );

}

void Channel::rememberCreationTime() {

    mCreationTime = posix::second_clock::universal_time();
}

const Timestamp Channel::creationTime() const {

    return mCreationTime;
}

const uint16_t Channel::expectedPacketsCount() const {

    return mExpectedPacketsCount;
}

const uint16_t Channel::realPacketsCount() const {

    return (uint16_t) mPackets->size();
}

void Channel::setOutgoingPacketsCount(
    uint16_t packetsCount) {

    mOutgoingPacketsCount = packetsCount;
}

bool Channel::increaseSendedPacketsCounter() {

    mSendedPacketsCount += 1;
    return mOutgoingPacketsCount == mSendedPacketsCount;
}

const map<uint16_t, Packet::Shared> *Channel::packets() const {

    return mPackets;
}

const uint16_t Channel::kCRCPacketNumber() {

    static uint16_t crcPacketNumber = 0;
    return crcPacketNumber;
}