/**
 * This file is part of GEO Protocol.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Protocol/GEO-network-client/blob/master/LICENSE.md
 *
 * No part of GEO Protocol, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#ifndef GEO_NETWORK_CLIENT_INCOMINGCHANNEL_H
#define GEO_NETWORK_CLIENT_INCOMINGCHANNEL_H

#include "MessageParser.h"
#include "../common/Types.h"
#include "../common/Packet.hpp"

#include "../../../messages/Message.hpp"
#include "../../../../common/memory/MemoryUtils.h"
#include "../../../../common/exceptions/ConflictError.h"

#include <boost/unordered_map.hpp>
#include <boost/crc.hpp>

#include <utility>
#include <limits>
#include <chrono>


using namespace std;


/**
 * Collects incoming packets from the remote node.
 */
class IncomingChannel {
public:
    using Unique = unique_ptr<IncomingChannel>;

public:
    IncomingChannel(
        MessagesParser &messageParser,
        TimePoint &nodeHandlerLastUpdate,
        Logger &logger)
        noexcept;

    ~IncomingChannel()
        noexcept;

    void clear()
        noexcept;

    void reservePacketsSlots(
        const Packet::Count count)
        noexcept(false);

    void addPacket(
        const PacketHeader::PacketIndex index,
        byte* bytes,
        const PacketHeader::PacketSize count)
        noexcept(false);

    pair<bool, Message::Shared> tryCollectMessage();

    Packet::Size receivedPacketsCount() const
        noexcept;

    Packet::Size expectedPacketsCount() const
        noexcept;

    const TimePoint& lastUpdated() const
        noexcept;

protected:
    TimePoint mLastPacketReceived;
    TimePoint &mLastRemoteNodeHandlerUpdated;

    MessagesParser &mMessagesParser;
    Logger &mLog;
    Packet::Size mExpectedPacketsCount;

    boost::unordered_map<PacketHeader::PacketIndex, pair<void*, PacketHeader::PacketSize>> mPackets;
};


#endif //GEO_NETWORK_CLIENT_INCOMINGCHANNEL_H
