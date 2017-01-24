#ifndef GEO_NETWORK_CLIENT_CHANNELSMANAGER_H
#define GEO_NETWORK_CLIENT_CHANNELSMANAGER_H

#include "../channel/Channel.h"

#include "../../../common/exceptions/IndexError.h"
#include "../../../common/exceptions/MemoryError.h"

#include <boost/asio.hpp>

#include <cstdint>
#include <map>

using namespace std;
using namespace boost::asio::ip;

class ChannelsManager {

public:
    ChannelsManager();

    ~ChannelsManager();

    pair<Channel::Shared, udp::endpoint> channel(
        uint16_t number,
        udp::endpoint endpoint) const;

    pair<Channel::Shared, udp::endpoint> create(
        uint16_t number,
        udp::endpoint endpoint) const;

    void remove(
        uint16_t number) const;

    uint16_t unusedChannelNumber(
        udp::endpoint endpoint
    );

private:
    // <number, channel>
    map<uint16_t, Channel::Shared> *mChannels;
    // <number, endpoint>
    map<uint16_t, udp::endpoint> *mEndpoints;
};


#endif //GEO_NETWORK_CLIENT_CHANNELSMANAGER_H
