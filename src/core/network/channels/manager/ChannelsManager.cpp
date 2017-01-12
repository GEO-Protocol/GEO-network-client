#include "ChannelsManager.h"

ChannelsManager::ChannelsManager() {

    try {
        mChannels = new map<uint16_t, Channel::Shared>();

    } catch (std::bad_alloc &e) {
        throw MemoryError("ChannelsManager::ChannelsManager: "
                              "Can not allocate memory for channels container.");
    }

    try {
        mEndpoints = new map<uint16_t, udp::endpoint>();

    } catch (std::bad_alloc &e) {
        delete mEndpoints;
        throw MemoryError("ChannelsManager::ChannelsManager: "
                              "Can not allocate memory for endpoint container.");
    }
}

ChannelsManager::~ChannelsManager() {

    mChannels->clear();
    delete mChannels;
    mEndpoints->clear();
    delete mEndpoints;
}

pair<Channel::Shared, udp::endpoint> ChannelsManager::channel(
    uint16_t number,
    udp::endpoint endpoint) const {

    if (mChannels->count(number) != 0){
        return make_pair(
            mChannels->at(number),
            mEndpoints->at(number)
        );

    } else {
        return create(
            number,
            endpoint
        );
    }
}

pair<Channel::Shared, udp::endpoint> ChannelsManager::create(
    uint16_t number,
    udp::endpoint endpoint) const {

    Channel *channel = nullptr;
    try {
        channel = new Channel();

    } catch (std::bad_alloc &e) {
        throw MemoryError("ChannelsManager::create: "
                              "Can not allocate memory for channel instance.");
    }
    mChannels->insert(
        make_pair(
            number,
            Channel::Shared(channel)
        )
    );
    mEndpoints->insert(
        make_pair(
            number,
            endpoint
        )
    );

    return make_pair(
        mChannels->at(number),
        mEndpoints->at(number)
    );
}

void ChannelsManager::remove(
    uint16_t number) const {

    if (mChannels->count(number) != 0) {
        auto itChan = mChannels->find(number);
        mChannels->erase(itChan);
        auto itEnd = mEndpoints->find(number);
        mEndpoints->erase(itEnd);

    } else {
        throw IndexError("ChannelsManager::remove: "
                             "Channel with such number does not exist.");
    }
}

uint16_t ChannelsManager::unusedChannelNumber(
    udp::endpoint endpoint) {

    uint16_t lastUningNumber = 0;
    for (auto const &it : *mEndpoints) {
        if (endpoint == it.second) {
            lastUningNumber = it.first;
        }
    }
    return lastUningNumber != 0 ? lastUningNumber++ : 0;
}
