#ifndef GEO_NETWORK_CLIENT_PACKETHEADER_H
#define GEO_NETWORK_CLIENT_PACKETHEADER_H

#include <memory>
#include <cstdint>

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

    const uint16_t channel() const;

    const uint16_t packetNumber() const;

    const uint16_t totalPacketsCount() const;

    const uint16_t bytesCount() const;

private:
    uint16_t mChannel;
    uint16_t mPacketNumber;
    uint16_t mTotalPacketsCount;
    uint16_t mBytesCount;
};


#endif //GEO_NETWORK_CLIENT_PACKETHEADER_H
