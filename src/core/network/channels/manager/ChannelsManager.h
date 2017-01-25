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

    pair<Channel::Shared, udp::endpoint> incomingChannel(
        uint16_t number,
        udp::endpoint endpoint);

    pair<Channel::Shared, udp::endpoint> createIncomingChannel(
        uint16_t number,
        udp::endpoint endpoint);

    void removeIncomingChannel(
        uint16_t number);

    pair<uint16_t, Channel::Shared> outgoingChannel(
        udp::endpoint endpoint);

    Channel::Shared createOutgoingChannel(
        uint16_t number,
        udp::endpoint endpoint);

    void removeOutgoingChannel(
        uint16_t number);

    uint16_t unusedOutgoingChannelNumber(
        udp::endpoint endpoint
    );

private:
    // <number, channel>
    map<uint16_t, Channel::Shared> *mIncomingChannels;
    map<uint16_t, Channel::Shared> *mOutgoingChannels;

    // <number, endpoint>
    map<uint16_t, udp::endpoint> *mIncomingEndpoints;
    map<uint16_t, udp::endpoint> *mOutgoingEndpoints;
};


#endif //GEO_NETWORK_CLIENT_CHANNELSMANAGER_H
