#include "ChannelsManager.h"

ChannelsManager::ChannelsManager(
    as::io_service &ioService) :

    mIOService(ioService),
    mProcessingTimer(new as::steady_timer(mIOService)) {

    try {
        mIncomingChannels = unique_ptr<map<udp::endpoint, vector<pair<uint16_t, Channel::Shared>>>>(
            new map<udp::endpoint, vector<pair<uint16_t, Channel::Shared>>>());

        mOutgoingChannels = unique_ptr<map<udp::endpoint, pair<uint16_t, Channel::Shared>>>(
            new map<udp::endpoint, pair<uint16_t, Channel::Shared>>());

    } catch (std::bad_alloc &) {
        throw MemoryError("ChannelsManager::ChannelsManager: "
                              "Can not allocate memory for channels container.");
    }

    removeDeprecatedIncomingChannels();
}

pair<Channel::Shared, udp::endpoint> ChannelsManager::incomingChannel(
    const uint16_t number,
    const udp::endpoint &endpoint) {

    if (mIncomingChannels->count(endpoint) != 0){

        auto endpointAndVectorOfNumberAndChannelPairs = mIncomingChannels->find(endpoint);
        for (const auto &numberAndChannel : endpointAndVectorOfNumberAndChannelPairs->second) {

            if (number != numberAndChannel.first) {
                continue;
            }

            return make_pair(
                numberAndChannel.second,
                endpoint);

        }

        Channel::Shared channel = make_shared<Channel>();
        endpointAndVectorOfNumberAndChannelPairs->second.push_back(
            make_pair(
                number,
                channel)
        );

        return make_pair(
            channel,
            endpoint);

    } else {

        Channel::Shared channel = make_shared<Channel>();
        vector<pair<uint16_t, Channel::Shared>> numbersAndChannels;
        numbersAndChannels.reserve(1);
        numbersAndChannels.push_back(
            make_pair(
                number,
                channel));

        mIncomingChannels->insert(
            make_pair(
                endpoint,
                numbersAndChannels));

        return make_pair(
            channel,
            endpoint);

    }
}

void ChannelsManager::removeIncomingChannel(
    const udp::endpoint &endpoint,
    const uint16_t channelNumber) {

    if (mIncomingChannels->count(endpoint) != 0) {

        auto endpointAndVectorOfNumberAndChannelPairs = mIncomingChannels->find(endpoint);

        for (size_t position = 0; position < endpointAndVectorOfNumberAndChannelPairs->second.size(); ++ position) {

            if (channelNumber == endpointAndVectorOfNumberAndChannelPairs->second.at(position).first) {
                endpointAndVectorOfNumberAndChannelPairs->second.erase(
                    endpointAndVectorOfNumberAndChannelPairs->second.begin() + position);
            }

        }

    } else {
        throw IndexError("ChannelsManager::removeIncomingChannel: "
                             "Incoming channels for such endpoint does not exists.");
    }
}

pair<uint16_t, Channel::Shared> ChannelsManager::outgoingChannel(
    const udp::endpoint &endpoint) {

    uint16_t channelNumber = unusedOutgoingChannelNumber(
        endpoint);

    return make_pair(
        channelNumber,
        createOutgoingChannel(
            channelNumber,
            endpoint));
}

Channel::Shared ChannelsManager::createOutgoingChannel(
    const uint16_t number,
    const udp::endpoint &endpoint) {

    Channel::Shared channel = make_shared<Channel>();

    if (mOutgoingChannels->count(endpoint) != 0) {

        (*mOutgoingChannels)[endpoint] = make_pair(
            number,
            make_shared<Channel>());

    } else {
        mOutgoingChannels->insert(
            make_pair(
                endpoint,
                make_pair(
                    number,
                    make_shared<Channel>())));
    }

    return Channel::Shared(
        mOutgoingChannels->at(
            endpoint).second);
}


uint16_t ChannelsManager::unusedOutgoingChannelNumber(
    const udp::endpoint &endpoint) {

    uint16_t maximalPossibleOutgoingChannelNumber = std::numeric_limits<uint16_t>::max();

    if (mOutgoingChannels->count(endpoint) != 0) {

        const auto endpointAndChannel = mOutgoingChannels->find(
            endpoint);

        if (endpointAndChannel->second.first < maximalPossibleOutgoingChannelNumber) {

            uint16_t channelNumber = endpointAndChannel->second.first + 1;
            return channelNumber;
        }

    }

    return 0;
}

void ChannelsManager::removeOutgoingChannel(
    const udp::endpoint &endpoint) {

    if (mOutgoingChannels->count(endpoint) != 0) {

        auto itChannel = mOutgoingChannels->find(
            endpoint);
        mOutgoingChannels->erase(
            itChannel);

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

        for (const auto &endpointAndVectorOfNumberAndChannelPairs : *mIncomingChannels) {

            for (const auto &numberAndChannel : endpointAndVectorOfNumberAndChannelPairs.second) {

                if ((utc_now() - numberAndChannel.second->creationTime()) > kIncomingChannelKeepAliveTimeout()) {
                    removeIncomingChannel(
                        endpointAndVectorOfNumberAndChannelPairs.first,
                        numberAndChannel.first);

                }

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
