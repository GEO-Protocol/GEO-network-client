#include "Channel.h"

Channel::Channel() {

    try {
        mPackets = unique_ptr<map<uint16_t, Packet::Shared, less<uint16_t>>> (new map<uint16_t, Packet::Shared, less<uint16_t>>());

    } catch (std::bad_alloc&) {
        throw MemoryError("Channel::Channel: "
                              "Can not allocate memory for packets container.");
    }
    rememberCreationTime();
}

Channel::~Channel() {}

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
            packet));

}

bool Channel::checkConsistency() {

    if (mPackets->size() == mExpectedPacketsCount) {

        if (mPackets->count(kCRCPacketNumber()) != 0) {

            uint32_t *controlSum = new(const_cast<byte *> (mPackets->at(kCRCPacketNumber())->body().get())) uint32_t;

            auto channelBytesAndCount = data();
            boost::crc_32_type control;
            control.process_bytes(
                channelBytesAndCount.first.get(),
                channelBytesAndCount.second);

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

    BytesShared dataShared = tryCalloc(
        totalBytesCount);

    size_t nextPacketBytesOffset = 0;
    for (auto &numberAndChannel : *mPackets) {

        if (numberAndChannel.first != kCRCPacketNumber()) {

            memcpy(
                dataShared.get() + nextPacketBytesOffset,
                const_cast<byte *>(numberAndChannel.second->body().get()),
                (size_t) numberAndChannel.second->header()->bodyBytesCount());

            nextPacketBytesOffset += numberAndChannel.second->header()->bodyBytesCount();

        }

    }

    return make_pair(
        dataShared,
        totalBytesCount
    );
}

void Channel::rememberCreationTime() {

    mCreationTime = utc_now();
}

const DateTime Channel::creationTime() const {

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

bool Channel::canBeRemoved () {

    mSentPacketsCount += 1;
    return mOutgoingPacketsCount == mSentPacketsCount;
}

const map<uint16_t, Packet::Shared> *Channel::packets() const {

    return mPackets.get();
}

const uint16_t Channel::kCRCPacketNumber() {

    static uint16_t crcPacketNumber = 0;
    return crcPacketNumber;
}
