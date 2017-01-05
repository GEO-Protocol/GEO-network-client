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

    if (mChannels->count(number) > 0){
        return mChannels->at(number);

    } else {
        return create(number);
    }
}

Channel::Shared ChannelsManager::create(
    uint16_t number) const {

    mChannels->insert(
        make_pair(
            number,
            Channel::Shared(new Channel())
        )
    );

    return mChannels->at(number);
}

void ChannelsManager::remove(
    uint16_t number) const {

    if (mChannels->count(number) > 0) {
        auto it = mChannels->find(number);
        mChannels->erase(it);

    } else {
        throw IndexError("ChannelsManager::remove: "
                             "Channel with such number does not exist.");
    }
}
