#ifndef GEO_NETWORK_CLIENT_CHANNEL_H
#define GEO_NETWORK_CLIENT_CHANNEL_H

#include "../../../common/Types.h"

#include "../packet/Packet.h"

#include "../../../common/exceptions/ConflictError.h"

#include <memory>
#include <malloc.h>
#include <string>
#include <cstring>
#include <cstdint>
#include <map>

using namespace std;

class Channel {
public:
    typedef shared_ptr<Channel> Shared;

public:
    Channel();

    ~Channel();

    void addPacket(
        uint16_t position,
        Packet::Shared packet) const;

    bool checkConsistency() const;

    ConstBytesShared data() const;

private:
    map<uint16_t, Packet::Shared, less<uint16_t>> *mPackets;
};


#endif //GEO_NETWORK_CLIENT_CHANNEL_H
