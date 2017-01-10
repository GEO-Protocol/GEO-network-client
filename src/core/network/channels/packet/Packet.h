#ifndef GEO_NETWORK_CLIENT_PACKET_H
#define GEO_NETWORK_CLIENT_PACKET_H

#include "../../../common/Types.h"
#include "PacketHeader.h"

#include <cstring>
#include <memory>
#include <malloc.h>
#include <cstdint>

using namespace std;

class Packet {

public:
    typedef shared_ptr<Packet> Shared;
    typedef shared_ptr<const Packet> SharedConst;

public:
    Packet(
        PacketHeader *packetHeader,
        byte *bytes,
        size_t bytesCount);

    ~Packet();

    PacketHeader::SharedConst header() const;

    ConstBytesShared body() const;

public:
    static const constexpr size_t kChannelNumberOffset = PacketHeader::kHeaderRecordSize;
    static const constexpr size_t kPackageNumberOffset = PacketHeader::kHeaderRecordSize * 2;
    static const constexpr size_t kTotalPacketsCountOffset = PacketHeader::kHeaderRecordSize * 3;
    static const constexpr size_t kPacketBodyOffset = PacketHeader::kHeaderSize;

private:
    PacketHeader::Shared mPacketHeader;
    BytesShared mBytes;
};


#endif //GEO_NETWORK_CLIENT_PACKET_H
