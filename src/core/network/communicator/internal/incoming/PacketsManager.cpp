#include "PacketsManager.h"


PacketsManager::MemoryPacket& PacketsManager::packet(
    PacketHeader::ChannelIndex channelIndex,
    PacketHeader::PacketIndex packetIndex)
{
    const size_t kPacketPosition = (channelIndex * numeric_limits<PacketHeader::PacketIndex>::max()) + packetIndex;
    if (mPackets.count(kPacketPosition) == 0) {
        mPackets[kPacketPosition] = MemoryPacket();
    }

    return mPackets[kPacketPosition];
}

void PacketsManager::dropEntireChannel(
    PacketHeader::ChannelIndex channelIndex)
{
    // Prevent map elements reallocation
    mPackets.reserve(mPackets.size());

    size_t packetPosition = (channelIndex * numeric_limits<PacketHeader::PacketIndex>::max());
    while (mPackets.count(packetPosition) != 0) {
        mPackets.erase(packetPosition);
        ++packetPosition;
    }

    // Reallocate elements and free memory
    mPackets.shrink_to_fit();
}

void PacketsManager::reserveSlotsForChannel(
    PacketHeader::TotalPacketsCount packetsCount)
{
    mPackets.reserve(
        mPackets.capacity() + packetsCount);
}
