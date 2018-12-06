#ifndef GEO_NETWORK_CLIENT_OUTGOINGREMOTEADDRESSNODE_H
#define GEO_NETWORK_CLIENT_OUTGOINGREMOTEADDRESSNODE_H

#include "../common/Types.h"
#include "../common/Packet.hpp"

#include "../../../../contractors/addresses/BaseAddress.h"
#include "../../../messages/Message.hpp"

#include "../../../../common/memory/MemoryUtils.h"
#include "../../../../logger/Logger.h"
#include "../../../../common/exceptions/Exception.h"

#include <boost/crc.hpp>
#include <boost/asio/steady_timer.hpp>
#include <queue>

namespace as = boost::asio;

class OutgoingRemoteAddressNode {

public:
    using Shared = shared_ptr<OutgoingRemoteAddressNode>;
    using Unique = unique_ptr<OutgoingRemoteAddressNode>;

public:
    OutgoingRemoteAddressNode(
        BaseAddress::Shared address,
        UDPSocket &socket,
        IOService &ioService,
        Logger &logger)
        noexcept;

    void sendMessage(
        Message::Shared message)
        noexcept;

    bool containsPacketsInQueue() const;

protected:
    uint32_t crc32Checksum(
        byte* data,
        size_t bytesCount)
        const noexcept;

    void populateQueueWithNewPackets(
        byte* messageData,
        const size_t bytesCount);

    void beginPacketsSending();

    PacketHeader::ChannelIndex nextChannelIndex()
    noexcept;

    LoggerStream errors() const;

    LoggerStream debug() const;

protected:
    BaseAddress::Shared mAddress;

    IOService &mIOService;
    UDPSocket &mSocket;
    Logger &mLog;

    queue<pair<byte*, Packet::Size>> mPacketsQueue;
    PacketHeader::ChannelIndex mNextAvailableChannelIndex;

    // This pair contains date time of last packet sending
    // and count of sending operations, that would be done in interval, less than 50 msecs between 2 operations.
    pair<boost::posix_time::ptime, size_t> mCyclesStats;
    as::deadline_timer mSendingDelayTimer;
};


#endif //GEO_NETWORK_CLIENT_OUTGOINGREMOTEADDRESSNODE_H
