#include "ChannelsManager.h"

ChannelsManager::ChannelsManager() {

    mChannels = new map<uint16_t, Channel::Shared>();
}

ChannelsManager::~ChannelsManager() {

    mChannels->clear();
    delete mChannels;
}

Channel::Shared ChannelsManager::channel(
    uint16_t number) const {

    if (mChannels->count(number) != 0){
        return mChannels->at(number);

    } else {
        return create(number);
    }
}

Channel::Shared ChannelsManager::create(
    uint16_t number) const {

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

    return mChannels->at(number);
}

void ChannelsManager::remove(
    uint16_t number) const {

    if (mChannels->count(number) != 0) {
        auto it = mChannels->find(number);
        mChannels->erase(it);

    } else {
        throw IndexError("ChannelsManager::remove: "
                             "Channel with such number does not exist.");
    }
}
