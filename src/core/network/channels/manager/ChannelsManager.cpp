#include "ChannelsManager.h"

ChannelsManager::ChannelsManager(
    as::io_service &ioService) :

    mIOService(ioService){

    try {
        mProcessingTimer = unique_ptr<as::deadline_timer>(
            new as::deadline_timer(
                mIOService,
                boost::posix_time::milliseconds(2 * 1000)
            )
        );

    } catch (std::bad_alloc &) {
        throw MemoryError("ChannelsManager::ChannelsManager: "
                              "Can not allocate memory for deadline timer instance.");
    }

    try {
        mIncomingChannels = unique_ptr<map<uint16_t, Channel::Shared>>(
            new map<uint16_t, Channel::Shared>()
        );
        mOutgoingChannels = unique_ptr<map<uint16_t, Channel::Shared>>(
            new map<uint16_t, Channel::Shared>()
        );

    } catch (std::bad_alloc &) {
        throw MemoryError("ChannelsManager::ChannelsManager: "
                              "Can not allocate memory for channels container.");
    }

    try {
        mIncomingEndpoints = unique_ptr<map<uint16_t, udp::endpoint>>(
            new map<uint16_t, udp::endpoint>()
        );
        mOutgoingEndpoints = unique_ptr<map<uint16_t, udp::endpoint>>(
            new map<uint16_t, udp::endpoint>()
        );

    } catch (std::bad_alloc &) {
        throw MemoryError("ChannelsManager::ChannelsManager: "
                              "Can not allocate memory for endpoint container.");
    }

    removeDeprecatedIncomingChannels();
}

ChannelsManager::~ChannelsManager() {}

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

    } catch (std::bad_alloc &) {
        throw MemoryError("ChannelsManager::createIncomingChannel: "
                              "Can not allocate memory for incoming channel instance.");
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
        auto itChannel = mIncomingChannels->find(number);
        mIncomingChannels->erase(itChannel);
        auto itEndpoint = mIncomingEndpoints->find(number);
        mIncomingEndpoints->erase(itEndpoint);

    } else {
        throw IndexError("ChannelsManager::removeIncomingChannel: "
                             "Channel with such number does not exist.");
    }
}

pair<uint16_t, Channel::Shared> ChannelsManager::outgoingChannel(
    udp::endpoint endpoint) {

    uint16_t number = unusedOutgoingChannelNumber();
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

    } catch (std::bad_alloc &) {
        throw MemoryError("ChannelsManager::createOutgoingChannel: "
                              "Can not allocate memory for outgoing channel instance.");
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
        auto itChannel = mOutgoingChannels->find(number);
        mOutgoingChannels->erase(itChannel);
        auto itEndpoint = mOutgoingEndpoints->find(number);
        mOutgoingEndpoints->erase(itEndpoint);

    } else {
        throw IndexError("ChannelsManager::removeOutgoingChannel: "
                             "Channel with such number does not exist.");
    }
}

uint16_t ChannelsManager::unusedOutgoingChannelNumber() {

    uint16_t minimalRange = 0;
    uint16_t maximalRange = 0;
    for (auto const &numberAndEndpoint : *mOutgoingChannels) {
        if (numberAndEndpoint.first > maximalRange) {
            maximalRange = numberAndEndpoint.first;
        }
    }
    for (uint16_t number = minimalRange; number < maximalRange; ++ number) {
        if (mOutgoingChannels->count(number) == 0) {
            return number;
        }
    }
    return maximalRange += 1;
}

void ChannelsManager::removeDeprecatedIncomingChannels() {

    mProcessingTimer->cancel();
    mProcessingTimer->expires_from_now(kIncomingChannelsCollectorTimeout());
    mProcessingTimer->async_wait(
        boost::bind(
            &ChannelsManager::handleIncomingChannelsCollector,
            this,
            as::placeholders::error
        )
    );
}

void ChannelsManager::handleIncomingChannelsCollector(
    const boost::system::error_code &error) {

    if (!error) {
        for (auto const &numberAndChannel : *mIncomingChannels) {
            if ((posix::microsec_clock::universal_time() - numberAndChannel.second->creationTime()) > kIncomingChannelKeepAliveTimeout()) {
                removeIncomingChannel(numberAndChannel.first);
            }
        }
    }
}

const Duration ChannelsManager::kIncomingChannelsCollectorTimeout() {

    static const Duration timeout = posix::hours(1);
    return timeout;
}

const Duration ChannelsManager::kIncomingChannelKeepAliveTimeout() {

    static const Duration timeout = posix::minutes(1);
    return timeout;
}
