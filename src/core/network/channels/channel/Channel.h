#ifndef GEO_NETWORK_CLIENT_CHANNEL_H
#define GEO_NETWORK_CLIENT_CHANNEL_H

#include "../../../common/Types.h"

#include "../packet/Packet.h"

#include "../../../common/exceptions/ConflictError.h"
#include "../../../common/exceptions/MemoryError.h"

#include <boost/crc.hpp>

#include <malloc.h>
#include <memory>
#include <map>
#include <string>
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

    pair<ConstBytesShared, size_t> data();

    const Timestamp creationTime() const;

    const uint16_t expectedPacketsCount() const;

    const uint16_t realPacketsCount() const;

    void setOutgoingPacketsCount(
        uint16_t packetsCount);

    bool increaseSendedPacketsCounter();

    const map<uint16_t, Packet::Shared, less<uint16_t>> *packets() const;

    static const uint16_t kCRCPacketNumber();

private:
    void rememberCreationTime();

private:
    Timestamp mCreationTime;

    uint16_t mExpectedPacketsCount;
    uint16_t mOutgoingPacketsCount;
    uint16_t mSendedPacketsCount;

    map<uint16_t, Packet::Shared, less<uint16_t>> *mPackets;
};


#endif //GEO_NETWORK_CLIENT_CHANNEL_H
