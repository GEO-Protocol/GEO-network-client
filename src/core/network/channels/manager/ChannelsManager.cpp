#include "ChannelsManager.h"

ChannelsManager::ChannelsManager() {

    try {
        mIncomingChannels = new map<uint16_t, Channel::Shared>();
        mOutgoingChannels = new map<uint16_t, Channel::Shared>();

    } catch (std::bad_alloc &e) {
        throw MemoryError("ChannelsManager::ChannelsManager: "
                              "Can not allocate memory for channels container.");
    }

    try {
        mIncomingEndpoints = new map<uint16_t, udp::endpoint>();
        mOutgoingEndpoints = new map<uint16_t, udp::endpoint>();

    } catch (std::bad_alloc &e) {
        delete mIncomingEndpoints;
        throw MemoryError("ChannelsManager::ChannelsManager: "
                              "Can not allocate memory for endpoint container.");
    }
}

ChannelsManager::~ChannelsManager() {

    mIncomingChannels->clear();
    mOutgoingChannels->clear();
    delete mIncomingChannels;
    delete mOutgoingChannels;
    mIncomingEndpoints->clear();
    mOutgoingEndpoints->clear();
    delete mIncomingEndpoints;
    delete mOutgoingEndpoints;
}

pair<Channel::Shared, udp::endpoint> ChannelsManager::incomingChannel(
    uint16_t number,
    udp::endpoint endpoint) {

    if (mIncomingChannels->count(number) != 0){
        return make_pair(
            mIncomingChannels->at(number),
            mIncomingEndpoints->at(number)
        );

    } else {
        return createIncomingChannel(
            number,
            endpoint
        );
    }
}

pair<Channel::Shared, udp::endpoint> ChannelsManager::createIncomingChannel(
    uint16_t number,
    udp::endpoint endpoint) {

    Channel *channel = nullptr;
    try {
        channel = new Channel();

    } catch (std::bad_alloc &e) {
        throw MemoryError("ChannelsManager::create: "
                              "Can not allocate memory for incomingChannel instance.");
    }
    mIncomingChannels->insert(
        make_pair(
            number,
            Channel::Shared(channel)
        )
    );
    mIncomingEndpoints->insert(
        make_pair(
            number,
            endpoint
        )
    );

    return make_pair(
        mIncomingChannels->at(number),
        mIncomingEndpoints->at(number)
    );
}

void ChannelsManager::removeIncomingChannel(
    uint16_t number) {

    if (mIncomingChannels->count(number) != 0) {
        auto itChan = mIncomingChannels->find(number);
        mIncomingChannels->erase(itChan);
        auto itEnd = mIncomingEndpoints->find(number);
        mIncomingEndpoints->erase(itEnd);

    } else {
        throw IndexError("ChannelsManager::removeIncomingChannel: "
                             "Channel with such number does not exist.");
    }
}

pair<uint16_t, Channel::Shared> ChannelsManager::outgoingChannel(
    udp::endpoint endpoint) {

    for (const auto &numberAndChannel : *mOutgoingChannels) {
        if ((posix::second_clock::universal_time() - numberAndChannel.second->sendTime()) > posix::seconds(3)) {
            uint16_t number = numberAndChannel.first;
            removeOutgoingChannel(number);
            return make_pair(
                number,
                createOutgoingChannel(
                    number,
                    endpoint
                )
            );
        }
    }
    uint16_t number = unusedOutgoingChannelNumber(endpoint);
    return make_pair(
        number,
        createOutgoingChannel(
            number,
            endpoint
        )
    );
}

Channel::Shared ChannelsManager::createOutgoingChannel(
    uint16_t number,
    udp::endpoint endpoint) {

    Channel *channel = nullptr;
    try {
        channel = new Channel();

    } catch (std::bad_alloc &e) {
        throw MemoryError("ChannelsManager::create: "
                              "Can not allocate memory for incomingChannel instance.");
    }
    mOutgoingChannels->insert(
        make_pair(
            number,
            Channel::Shared(channel)
        )
    );
    mOutgoingEndpoints->insert(
        make_pair(
            number,
            endpoint
        )
    );

    return mOutgoingChannels->at(number);
}

void ChannelsManager::removeOutgoingChannel(
    uint16_t number) {

    if (mOutgoingChannels->count(number) != 0) {
        auto itChan = mOutgoingChannels->find(number);
        mOutgoingChannels->erase(itChan);
        auto itEnd = mOutgoingEndpoints->find(number);
        mOutgoingEndpoints->erase(itEnd);

    } else {
        throw IndexError("ChannelsManager::removeOutgoingChannel: "
                             "Channel with such number does not exist.");
    }
}

uint16_t ChannelsManager::unusedOutgoingChannelNumber(
    udp::endpoint endpoint) {

    if (!mOutgoingEndpoints->empty()) {

        uint16_t number = 0;
        for (auto const &numberAndEndpoint : *mOutgoingEndpoints) {
            if (endpoint == numberAndEndpoint.second) {
                number = numberAndEndpoint.first;
            }
        }

        for (auto const &numberAndEndpoint : *mOutgoingEndpoints) {
            if (numberAndEndpoint.first > number) {
                number = numberAndEndpoint.first;
            }
        }
        number += 1;
        return number;

    } else {
        return 0;
    }

}
