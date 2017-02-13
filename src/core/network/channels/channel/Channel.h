#ifndef GEO_NETWORK_CLIENT_CHANNEL_H
#define GEO_NETWORK_CLIENT_CHANNEL_H

#include "../packet/Packet.h"

#include "../../../common/Types.h"
#include "../../../common/time/TimeUtils.h"
#include "../../../common/memory/MemoryUtils.h"

#include "../../../common/exceptions/ConflictError.h"
#include "../../../common/exceptions/MemoryError.h"

#include <boost/crc.hpp>

#include <map>
#include <string>
#include <memory>
#include <cstring>
#include <cstdint>

using namespace std;

class Channel {
public:
    typedef shared_ptr<Channel> Shared;

public:
    Channel();

    ~Channel();

    void addPacket(
        uint16_t position,
        Packet::Shared packet);

    bool checkConsistency();

    pair<BytesShared, size_t> data();

    const DateTime creationTime() const;

    const uint16_t expectedPacketsCount() const;

    const uint16_t realPacketsCount() const;

    void setOutgoingPacketsCount(
        uint16_t packetsCount);

    bool increaseSentPacketsCounter();

    const map<uint16_t, Packet::Shared, less<uint16_t>> *packets() const;

    static const uint16_t kCRCPacketNumber();

private:
    void rememberCreationTime();

private:
    DateTime mCreationTime;

    uint16_t mExpectedPacketsCount = 0;
    uint16_t mOutgoingPacketsCount = 0;
    uint16_t mSentPacketsCount = 0;

    unique_ptr<map<uint16_t, Packet::Shared, less<uint16_t>>> mPackets;
};


#endif //GEO_NETWORK_CLIENT_CHANNEL_H
