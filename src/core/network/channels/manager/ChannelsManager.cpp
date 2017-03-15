#include "ChannelsManager.h"

ChannelsManager::ChannelsManager(
    as::io_service &ioService,
    Logger *logger) :

    mIOService(ioService),
    mLog(logger),
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

    /*mLog->logInfo("ChannelsManager::incomingChannel: ",
                  "Channel for endpoint " + endpoint.address().to_string() + " : " + to_string(endpoint.port()));
    mLog->logInfo("ChannelsManager::incomingChannel: ",
                  "Channel number " + to_string(number));
    mLog->logInfo("ChannelsManager::incomingChannel: ",
                  "Endpoints map size " + mIncomingChannels->size());*/

    if (mIncomingChannels->count(endpoint) != 0){

        /*mLog->logInfo("ChannelsManager::incomingChannel: ",
                      "Endpoint already exist, find his channels set.");*/

        auto endpointAndVectorOfNumberAndChannelPairs = mIncomingChannels->find(
            endpoint);

        for (const auto &numberAndChannel : endpointAndVectorOfNumberAndChannelPairs->second) {

            if (number != numberAndChannel.first) {
                continue;
            }

            /*mLog->logInfo("ChannelsManager::incomingChannel: ",
                          "Channel with such number is exist, return channel by number.");*/

            return make_pair(
                numberAndChannel.second,
                endpoint);

        }

        /*mLog->logInfo("ChannelsManager::incomingChannel: ",
                      "Channel with such number does not exist, create new channel by number.");*/

        Channel::Shared channel = make_shared<Channel>();
        endpointAndVectorOfNumberAndChannelPairs->second.push_back(
            make_pair(
                number,
                channel));

        return make_pair(
            channel,
            endpoint);

    } else {

        /*mLog->logInfo("ChannelsManager::incomingChannel: ",
                      "Create new endpoint and his channels set.");*/

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

        /*mLog->logInfo("ChannelsManager::removeIncomingChannel: ",
                      "Remove channel by endpoint " + endpoint.address().to_string() + " : " + to_string(endpoint.port()));
        mLog->logInfo("ChannelsManager::removeIncomingChannel: ",
                      "Remove channel by number " + to_string(channelNumber));*/

        auto endpointAndVectorOfNumberAndChannelPairs = mIncomingChannels->find(
            endpoint);

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

    /*mLog->logInfo("ChannelsManager::outgoingChannel: ",
                  "Create channel for endpoint " + endpoint.address().to_string() + " : " + to_string(endpoint.port()));*/

    uint16_t channelNumber = unusedOutgoingChannelNumber(
        endpoint);

    /*mLog->logInfo("ChannelsManager::outgoingChannel: ",
                  "Number for channel " + to_string(channelNumber));*/

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

        /*mLog->logInfo("ChannelsManager::createOutgoingChannel: ",
                      "Endpoint already exist, create new channel.");*/

        (*mOutgoingChannels)[endpoint].first = number;
        (*mOutgoingChannels)[endpoint].second = channel;

    } else {
        /*mLog->logInfo("ChannelsManager::createOutgoingChannel: ",
                      "Endpoint does not exist yet.");*/

        mOutgoingChannels->insert(
            make_pair(
                endpoint,
                make_pair(
                    number,
                    channel)));
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

        /*mLog->logInfo("ChannelsManager::removeOutgoingChannel: ",
                      "Remove channel by endpoint " + endpoint.address().to_string() + " : " + to_string(endpoint.port()));*/

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
