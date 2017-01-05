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
    Packet::Shared packet) const {

    if (mPackets->count(position) > 0) {
        throw ConflictError("Channel::addPacket: "
                         "Packet with same position is already exist.");
    }

    mPackets->insert(
        make_pair(
            position,
            packet
        )
    );

}

bool Channel::checkConsistency() const {
    return mPackets->size() == mPackets->at(0)->header()->totalPacketsCount();
}

ConstBytesShared Channel::data() const {

    size_t totalBytesCount = 0;
    for (auto &it : *mPackets) {
        totalBytesCount = (size_t) it.second->header()->bytesCount();
    }

    totalBytesCount = totalBytesCount - 8 * mPackets->size();  // total bytes count minus header size
    byte *data = (byte *) malloc(totalBytesCount);
    memset(
        data,
        0,
        totalBytesCount
    );

    for (auto &it : *mPackets) {
        memcpy(
            data,
            it.second->body(),
            it.second->header()->bytesCount() - 8
        ); // bytes count minus header size
    }

    return ConstBytesShared(data, free);

}