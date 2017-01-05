#ifndef GEO_NETWORK_CLIENT_CHANNELSMANAGER_H
#define GEO_NETWORK_CLIENT_CHANNELSMANAGER_H

#include "../channel/Channel.h"

#include "../../../common/exceptions/IndexError.h"

#include <cstdint>
#include <map>

using namespace std;

class ChannelsManager {

public:
    ChannelsManager();

    ~ChannelsManager();

    Channel::Shared channel(
        uint16_t number) const;

    Channel::Shared create(
        uint16_t number) const;

    void remove(
        uint16_t number) const;

private:
    // <number, channel>
    map<uint16_t, Channel::Shared> *mChannels;
};


#endif //GEO_NETWORK_CLIENT_CHANNELSMANAGER_H
