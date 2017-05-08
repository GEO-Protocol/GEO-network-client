#ifndef GEO_NETWORK_CLIENT_INCOMINGCHANNEL_H
#define GEO_NETWORK_CLIENT_INCOMINGCHANNEL_H

#include "../common/Types.h"
#include "../common/Packet.hpp"
#include "MessageParser.h"

#include "../../../messages/Message.hpp"
#include "../../../../common/memory/MemoryUtils.h"
#include "../../../../common/exceptions/ConflictError.h"

#include <boost/unordered_map.hpp>

#include <utility>
#include <limits>
#include <chrono>


using namespace std;


class IncomingChannel {
public:
    using Unique = unique_ptr<IncomingChannel>;

public:
    IncomingChannel(
        MessagesParser &messageParser,
        TimePoint &nodeHandlerLastUpdate)
        noexcept;

    ~IncomingChannel();

    void clear()
        noexcept;

    void reserveSlots(
        const Packet::Count count);

    void addPacket(
        const PacketHeader::PacketIndex index,
        byte* bytes,
        const PacketHeader::PacketSize count);

    pair<bool, Message::Shared> tryCollectMessage();

    Packet::Size receivedPacketsCount() const;

    Packet::Size expectedPacketsCount() const;

    const TimePoint& lastUpdated() const;

protected:
    Packet::Size mExpectedPacketsCount;

    boost::unordered_map<PacketHeader::PacketIndex, pair<void*, PacketHeader::PacketSize>> mPackets;

    TimePoint mLastUpdated;
    TimePoint &mLastRemoteNodeHandlerUpdated;

    MessagesParser &mMessagesParser;
};


#endif //GEO_NETWORK_CLIENT_INCOMINGCHANNEL_H
