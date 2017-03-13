#include "ChannelsManager.h"

ChannelsManager::ChannelsManager(
    as::io_service &ioService) :

    mIOService(ioService),
    mProcessingTimer(new as::steady_timer(mIOService)) {

    try {
        mIncomingChannels = unique_ptr<map<uint16_t, Channel::Shared>>(
            new map<uint16_t, Channel::Shared>());

        mOutgoingChannels = unique_ptr<map<uint16_t, Channel::Shared>>(
            new map<uint16_t, Channel::Shared>());

    } catch (std::bad_alloc &) {
        throw MemoryError("ChannelsManager::ChannelsManager: "
                              "Can not allocate memory for channels container.");
    }

    try {
        mIncomingEndpoints = unique_ptr<map<uint16_t, udp::endpoint>>(
            new map<uint16_t, udp::endpoint>());

        mOutgoingEndpoints = unique_ptr<map<uint16_t, udp::endpoint>>(
            new map<uint16_t, udp::endpoint>());

    } catch (std::bad_alloc &) {
        throw MemoryError("ChannelsManager::ChannelsManager: "
                              "Can not allocate memory for endpoint container.");
    }

    removeDeprecatedIncomingChannels();
}

pair<Channel::Shared, udp::endpoint> ChannelsManager::incomingChannel(
    uint16_t number,
    udp::endpoint endpoint) {

    if (mIncomingChannels->count(number) != 0){
        return make_pair(
            mIncomingChannels->at(
                number),
            mIncomingEndpoints->at(
                number));

    } else {
        return createIncomingChannel(
            number,
            endpoint);
    }
}

pair<Channel::Shared, udp::endpoint> ChannelsManager::createIncomingChannel(
    uint16_t number,
    udp::endpoint endpoint) {

    Channel *channel = nullptr;
    try {
        channel = new Channel();

    } catch (std::bad_alloc &) {
        throw MemoryError("ChannelsManager::createIncomingChannel: "
                              "Can not allocate memory for incoming channel instance.");
    }
    mIncomingChannels->insert(
        make_pair(
            number,
            Channel::Shared(channel)));

    mIncomingEndpoints->insert(
        make_pair(
            number,
            endpoint));

    return make_pair(
        mIncomingChannels->at(
            number),
        mIncomingEndpoints->at(
            number));
}

void ChannelsManager::removeIncomingChannel(
    uint16_t number) {

    if (mIncomingChannels->count(number) != 0) {

        auto itChannel = mIncomingChannels->find(
            number);
        mIncomingChannels->erase(
            itChannel);

        auto itEndpoint = mIncomingEndpoints->find(
            number);
        mIncomingEndpoints->erase(
            itEndpoint);

    } else {
        throw IndexError("ChannelsManager::removeIncomingChannel: "
                             "Channel with such number does not exist.");
    }
}

pair<uint16_t, Channel::Shared> ChannelsManager::outgoingChannel(
    udp::endpoint endpoint) {

    unusedOutgoingChannelNumber();
    return make_pair(
        mNextOutgoingChannelNumber,
        createOutgoingChannel(
            mNextOutgoingChannelNumber,
            endpoint));
}

Channel::Shared ChannelsManager::createOutgoingChannel(
    uint16_t number,
    udp::endpoint endpoint) {

    Channel *channel = nullptr;
    try {
        channel = new Channel();

    } catch (std::bad_alloc &) {
        throw MemoryError("ChannelsManager::createOutgoingChannel: "
                              "Can not allocate memory for outgoing channel instance.");
    }

    if (mOutgoingChannels->count(number) != 0) {

        if (mOutgoingEndpoints->count(number) == 0) {
            throw ConflictError("ChannelsManager::createOutgoingChannel: "
                                 "Outgoing endpoint with such number does not exist when outgoing channel with such number is exist.");
        }

        (*mOutgoingChannels)[number] = Channel::Shared(channel);
        (*mOutgoingEndpoints)[number] = endpoint;

    } else {

        if (mOutgoingEndpoints->count(number) != 0) {
            throw ConflictError("ChannelsManager::createOutgoingChannel: "
                                 "Outgoing endpoint with such number is exist when outgoing channel with such number does not exist.");
        }

        mOutgoingChannels->insert(
            make_pair(
                number,
                Channel::Shared(channel)));

        mOutgoingEndpoints->insert(
            make_pair(
                number,
                endpoint));

    }

    return mOutgoingChannels->at(
        number);
}


void ChannelsManager::unusedOutgoingChannelNumber() {

    uint16_t maximalPossibleOutgoingChannelNumber = std::numeric_limits<uint16_t>::max();

    if (mNextOutgoingChannelNumber == maximalPossibleOutgoingChannelNumber) {

        mNextOutgoingChannelNumber = 0;

    } else {
        mNextOutgoingChannelNumber += 1;
    }

}

void ChannelsManager::removeOutgoingChannel(
    uint16_t number) {

    if (mOutgoingChannels->count(number) != 0) {

        auto itChannel = mOutgoingChannels->find(
            number);
        mOutgoingChannels->erase(
            itChannel);

        auto itEndpoint = mOutgoingEndpoints->find(
            number);
        mOutgoingEndpoints->erase(
            itEndpoint);

    } else {
        throw IndexError("ChannelsManager::removeOutgoingChannel: "
                             "Channel with such number does not exist.");
    }
}

void ChannelsManager::removeDeprecatedIncomingChannels() {

    mProcessingTimer->cancel();

    mProcessingTimer->expires_from_now(
        kIncomingChannelsCollectorTimeout());

    mProcessingTimer->async_wait(
        boost::bind(
            &ChannelsManager::handleIncomingChannelsCollector,
            this,
            as::placeholders::error));
}

void ChannelsManager::handleIncomingChannelsCollector(
    const boost::system::error_code &error) {

    if (!error) {
        for (auto const &numberAndChannel : *mIncomingChannels) {

            if ((utc_now() - numberAndChannel.second->creationTime()) > kIncomingChannelKeepAliveTimeout()) {
                removeIncomingChannel(
                    numberAndChannel.first);
            }

        }
    }
}

const chrono::hours ChannelsManager::kIncomingChannelsCollectorTimeout() {

    static const chrono::hours timeout = chrono::hours(1);
    return timeout;
}

const Duration ChannelsManager::kIncomingChannelKeepAliveTimeout() {

    static const Duration timeout = pt::minutes(1);
    return timeout;
}
