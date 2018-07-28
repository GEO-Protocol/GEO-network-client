#ifndef GEO_NETWORK_CLIENT_INCOMINGREMOTENODE_H
#define GEO_NETWORK_CLIENT_INCOMINGREMOTENODE_H

#include "../common/Types.h"
#include "../common/Packet.hpp"
#include "IncomingChannel.h"
#include "MessageParser.h"

#include <boost/unordered_map.hpp>

#include <vector>
#include <forward_list>


class IncomingRemoteNode {
public:
    using Unique = unique_ptr<IncomingRemoteNode>;

public:
    IncomingRemoteNode(
        const UDPEndpoint &endpoint,
        MessagesParser &messagesParser,
        Logger &logger)
        noexcept;

    void processIncomingBytesSequence(
        const byte *bytes,
        const size_t count)
        noexcept;

    bool tryCollectNextPacket();

    void dropEntireIncomingFlow();

    bool isBanned() const
        noexcept;

    Message::Shared popNextMessage();

    void dropOutdatedChannels();

    const UDPEndpoint& endpoint() const
        noexcept;

    const TimePoint& lastUpdated() const;

protected:
    IncomingChannel* findChannel (
        const PacketHeader::ChannelIndex index);

    LoggerStream debug() const
        noexcept;

protected:
    const UDPEndpoint mEndpoint;
    TimePoint mLastUpdated;

    boost::unordered_map<PacketHeader::ChannelIndex, IncomingChannel::Unique> mChannels;


    // todo: ensure reserve usage
    vector<byte> mBuffer;

    // It is expected, that incoming bytes flow from the network, may contains several messages at once.
    // There is non-zero probability, that whole bytes sequence would be processed in one read cycle,
    // so there are several messages, may be collected at once.
    //
    // This container stores messages, that was collected on previous read cycles.
    vector<Message::Shared> mCollectedMessages;

    MessagesParser &mMessagesParser;
    Logger &mLog;
};


#endif //GEO_NETWORK_CLIENT_INCOMINGREMOTENODE_H
