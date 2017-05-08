#ifndef PACKET_H
#define PACKET_H

#include "../../../../common/Types.h"


/**
 * Network packet scheme:
 *
 *   2B - Packet size;
 *   2B - Channel index;
 *   2B - Total packets count;
 *   1B - Current packet index;
 *   nB - Packet content, where n == (max packet size - packet header size)
 */
class PacketHeader {
public:
    typedef uint16_t PacketSize;
    typedef uint16_t ChannelIndex;
    typedef uint16_t TotalPacketsCount;

    // Note: one message may contains no more than 2**8 = 256 pakcets.
    // The max message size depends on max packet size:
    // for the public network (Internet transfer) the appropriate packet size is 508 bytes.
    // The packet header may contains 7 bytes. As the result, data segment of the packet may contains 501 byte.
    // The max message size = ~125kB.
    typedef uint8_t  PacketIndex;

    static const constexpr size_t kSize =
        sizeof(PacketSize)
      + sizeof(ChannelIndex)
      + sizeof(PacketIndex)
      + sizeof(TotalPacketsCount);


    static const uint16_t kPacketSizeOffset   = 0;
    static const uint16_t kChannelIndexOffset = kPacketSizeOffset     + sizeof(PacketSize);
    static const uint16_t kPacketsCountOffset = kChannelIndexOffset   + sizeof(ChannelIndex);
    static const uint16_t kPacketIndexOffset  = kPacketsCountOffset   + sizeof(TotalPacketsCount);
    static const uint16_t kDataOffset         = kPacketIndexOffset    + sizeof(PacketIndex);
};


class Packet {
public:
    typedef PacketHeader::PacketSize Size;
    typedef PacketHeader::TotalPacketsCount Count;
    typedef PacketHeader::TotalPacketsCount Index;

#ifdef ENGINE_TYPE_DC
    // In most cases, datacenter network is capable to process large UDP packets.
    // In case if engine is launched in the DC - there is much more efficient to use largest packet size,
    // and send less packets via the network.
    //
    // TODO: delivery problems may occur, on attempt ot send big packets to the decentralized network.
    // It would be great to be able to switch data packet size for each endpoint.
    static const Size kMaxSize = 2048;
#endif

#ifndef ENGINE_TYPE_DC
    // Some public network uses relatively small MTU (~600) bytes.
    // Some space in the packet must be reserved for the IP headers.
    //
    // For the details: http://stackoverflow.com/questions/1098897/what-is-the-largest-safe-udp-packet-size-on-the-internet
    static const Size kMaxSize = 508;
#endif

    static const constexpr Size kMinSize = PacketHeader::kSize + 1;
};

#endif // PACKET_H
