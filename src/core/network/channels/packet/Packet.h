#ifndef GEO_NETWORK_CLIENT_PACKET_H
#define GEO_NETWORK_CLIENT_PACKET_H

#include "../../../common/Types.h"
#include "../../../common/memory/MemoryUtils.h"

#include "PacketHeader.h"

#include <memory>
#include <cstring>
#include <cstdint>

using namespace std;

class Packet {

public:
    typedef shared_ptr<Packet> Shared;
    typedef shared_ptr<const Packet> SharedConst;

public:
    Packet(
        PacketHeader::Shared packetHeader,
        ConstBytesShared bytes);

    ~Packet();

    PacketHeader::SharedConst header() const;

    ConstBytesShared body() const;

    vector<byte> packetBytes() const;

public:
    static const constexpr size_t kChannelNumberOffset = PacketHeader::kHeaderRecordSize;
    static const constexpr size_t kPackageNumberOffset = PacketHeader::kHeaderRecordSize * 2;
    static const constexpr size_t kTotalPacketsCountOffset = PacketHeader::kHeaderRecordSize * 3;
    static const constexpr size_t kPacketBodyOffset = PacketHeader::kHeaderSize;

private:
    PacketHeader::SharedConst mPacketHeader;
    ConstBytesShared mBytes;
};


#endif //GEO_NETWORK_CLIENT_PACKET_H
