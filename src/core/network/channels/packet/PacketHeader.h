#ifndef GEO_NETWORK_CLIENT_PACKETHEADER_H
#define GEO_NETWORK_CLIENT_PACKETHEADER_H

#include "../../../common/Types.h"
#include "../../../common/memory/MemoryUtils.h"

#include <memory>
#include <cstdint>
#include <cstring>

using namespace std;

struct PacketHeader {

public:
    typedef shared_ptr<PacketHeader> Shared;
    typedef shared_ptr<const PacketHeader> SharedConst;

public:
    PacketHeader();

    PacketHeader(const uint16_t channel,
                 const uint16_t packetNumber,
                 const uint16_t totalPacketsCount,
                 const uint16_t bytesCount);

    ~PacketHeader();

    const uint16_t channelNumber() const;

    const uint16_t packetNumber() const;

    const uint16_t totalPacketsCount() const;

    const uint16_t packetBytesCount() const;

    const uint16_t bodyBytesCount() const;

    pair<ConstBytesShared, size_t> bytes() const;

public:
    static const constexpr size_t kHeaderRecordSize = 2;
    static const constexpr size_t kHeaderSize = kHeaderRecordSize * 4;

private:
    uint16_t mChannel;
    uint16_t mPacketNumber;
    uint16_t mTotalPacketsCount;
    uint16_t mPacketBytesCount;
    uint16_t mBodyBytesCount;
};


#endif //GEO_NETWORK_CLIENT_PACKETHEADER_H
