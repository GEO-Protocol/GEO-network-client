#ifndef GEO_NETWORK_CLIENT_CHANNELSMANAGER_H
#define GEO_NETWORK_CLIENT_CHANNELSMANAGER_H

#include "../channel/Channel.h"

#include "../../../common/Types.h"

#include "../../../common/exceptions/IndexError.h"
#include "../../../common/exceptions/MemoryError.h"
#include "../../../common/exceptions/ConflictError.h"

#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/bind.hpp>

#include <map>
#include <vector>
#include <utility>
#include <memory>
#include <chrono>
#include <stdint.h>

using namespace std;
using namespace boost::asio::ip;
namespace as = boost::asio;

class ChannelsManager {
public:
    ChannelsManager(
        as::io_service &ioService);

    pair<Channel::Shared, udp::endpoint> incomingChannel(
        const uint16_t number,
        const udp::endpoint &endpoint);

    void removeIncomingChannel(
        const udp::endpoint &endpoint,
        const uint16_t channelNumber);

    pair<uint16_t, Channel::Shared> outgoingChannel(
        const udp::endpoint &endpoint);

    void removeOutgoingChannel(
        const udp::endpoint &endpoint);

private:
    uint16_t unusedOutgoingChannelNumber(
        const udp::endpoint &endpoint);

    Channel::Shared createOutgoingChannel(
        const uint16_t number,
        const udp::endpoint &endpoint);

    void removeDeprecatedIncomingChannels();

    void handleIncomingChannelsCollector(
        const boost::system::error_code &error);

    static const chrono::hours kIncomingChannelsCollectorTimeout();

    static const Duration kIncomingChannelKeepAliveTimeout();

private:
    as::io_service &mIOService;
    unique_ptr<as::steady_timer> mProcessingTimer;

    // <endpoint, <channel number, channel>>
    unique_ptr<map<udp::endpoint, pair<uint16_t, Channel::Shared>>> mOutgoingChannels;
    unique_ptr<map<udp::endpoint, vector<pair<uint16_t, Channel::Shared>>>> mIncomingChannels;

};


#endif //GEO_NETWORK_CLIENT_CHANNELSMANAGER_H
