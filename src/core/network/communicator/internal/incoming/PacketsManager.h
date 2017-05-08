#ifndef PACKETSMANAGER_H
#define PACKETSMANAGER_H

#include "../../../../common/Types.h"
#include "../../../../common/exceptions/NotFoundError.h"

#include "../packet/packet.h"

#include <boost/container/flat_map.hpp>
#include <limits>


using namespace std;

class PacketsManager
{
public:
    struct MemoryPacket {
        byte data[numeric_limits<PacketHeader::PacketSize>::max()];
    };

public:
    MemoryPacket& packet(
        PacketHeader::ChannelIndex channelIndex,
        PacketHeader::PacketIndex packetIndex);

    void dropEntireChannel(
        PacketHeader::ChannelIndex channelIndex);

    void reserveSlotsForChannel(
        PacketHeader::TotalPacketsCount packetsCount);

protected:
    boost::container::flat_map<size_t, MemoryPacket> mPackets;
};

#endif // PACKETSMANAGER_H
