#ifndef GEO_NETWORK_CLIENT_CHANNELSMANAGER_H
#define GEO_NETWORK_CLIENT_CHANNELSMANAGER_H

#include "../channel/Channel.h"

#include "../../../common/Types.h"

#include "../../../common/exceptions/IndexError.h"
#include "../../../common/exceptions/MemoryError.h"

#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include <cstdint>
#include <map>
#include <memory>

using namespace std;
using namespace boost::asio::ip;
namespace as = boost::asio;

class ChannelsManager {
public:
    ChannelsManager(
        as::io_service &ioService);

    ~ChannelsManager();

    pair<Channel::Shared, udp::endpoint> incomingChannel(
        uint16_t number,
        udp::endpoint endpoint);

    void removeIncomingChannel(
        uint16_t number);

    pair<uint16_t, Channel::Shared> outgoingChannel(
        udp::endpoint endpoint);

    void removeOutgoingChannel(
        uint16_t number);

private:
    pair<Channel::Shared, udp::endpoint> createIncomingChannel(
        uint16_t number,
        udp::endpoint endpoint);

    Channel::Shared createOutgoingChannel(
        uint16_t number,
        udp::endpoint endpoint);

    uint16_t unusedOutgoingChannelNumber();

    void removeDeprecatedIncomingChannels();

    void handleIncomingChannelsCollector(
        const boost::system::error_code &error);

    static const Duration kIncomingChannelsCollectorTimeout();

    static const Duration kIncomingChannelKeepAliveTimeout();

private:
    as::io_service &mIOService;
    unique_ptr<as::deadline_timer> mProcessingTimer;

    // <number, channel>
    unique_ptr<map<uint16_t, Channel::Shared>> mIncomingChannels;
    unique_ptr<map<uint16_t, Channel::Shared>> mOutgoingChannels;

    // <number, endpoint>
    unique_ptr<map<uint16_t, udp::endpoint>> mIncomingEndpoints;
    unique_ptr<map<uint16_t, udp::endpoint>> mOutgoingEndpoints;
};


#endif //GEO_NETWORK_CLIENT_CHANNELSMANAGER_H
