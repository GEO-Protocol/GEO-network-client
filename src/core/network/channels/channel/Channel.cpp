#include "Channel.h"

Channel::Channel() {

    mPackets = new map<uint16_t, Packet::Shared, less<uint16_t>>();
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

            auto channelData = data();
            boost::crc_32_type control;
            control.process_bytes(
                channelData.first.get(),
                channelData.second
            );

            return control.checksum() == *controlSum;
        }
    }
    return false;
}

pair<ConstBytesShared, size_t> Channel::data() {

    size_t totalBytesCount = 0;
    for (auto &it : *mPackets) {
        if (it.first != kCRCPacketNumber()) {
            totalBytesCount += (size_t) it.second->header()->bodyBytesCount();
        }
    }

    byte *data = (byte *) malloc(totalBytesCount);
    memset(
        data,
        0,
        totalBytesCount
    );

    size_t nextPacketDataOffset = 0;
    for (auto &it : *mPackets) {
        if (it.first != kCRCPacketNumber()) {
            memcpy(
                data + nextPacketDataOffset,
                const_cast<byte *>(it.second->body().get()),
                (size_t) it.second->header()->bodyBytesCount()
            );
            nextPacketDataOffset += it.second->header()->bodyBytesCount();
        }
    }

    return make_pair(
        ConstBytesShared(
            data,
            free
        ),
        totalBytesCount
    );

}

const uint16_t Channel::expectedPacketsCount() const {

    return mExpectedPacketsCount;
}

const uint16_t Channel::realPacketsCount() const {

    return (uint16_t) mPackets->size();
}

const map<uint16_t, Packet::Shared> *Channel::packets() const {

    return mPackets;
}

void Channel::rememberSendTime() {

    mPacketsSendedTime = posix::second_clock::universal_time();
}

const Timestamp Channel::sendTime() const {

    return mPacketsSendedTime;
}

const uint16_t Channel::kCRCPacketNumber() {

    static uint16_t crcPacketNumber = 0;
    return crcPacketNumber;
}
