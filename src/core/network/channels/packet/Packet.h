#ifndef GEO_NETWORK_CLIENT_PACKET_H
#define GEO_NETWORK_CLIENT_PACKET_H

#include "../../../common/Types.h"
#include "PacketHeader.h"

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
        const byte *bytes);

    ~Packet();

    PacketHeader::SharedConst header() const;

    ConstBytesShared body() const;

private:
    PacketHeader::Shared mPacketHeader;
    BytesShared mBytes;
};


#endif //GEO_NETWORK_CLIENT_PACKET_H
