#ifndef GEO_NETWORK_CLIENT_OUTGOINGREMOTEBASENODE_H
#define GEO_NETWORK_CLIENT_OUTGOINGREMOTEBASENODE_H

#include "../common/Types.h"
#include "../common/Packet.hpp"

#include "../../../messages/Message.hpp"

#include "../../../../common/memory/MemoryUtils.h"
#include "../../../../logger/Logger.h"
#include "../../../../common/exceptions/Exception.h"

#include <boost/crc.hpp>
#include <boost/asio/steady_timer.hpp>
#include <queue>

namespace as = boost::asio;

class OutgoingRemoteBaseNode {
public:
    OutgoingRemoteBaseNode(
        UDPSocket &socket,
        IOService &ioService,
        Logger &logger);

    virtual ~OutgoingRemoteBaseNode();

    void sendMessage(
        pair<BytesShared, size_t>)
        noexcept;

    bool containsPacketsInQueue() const;

protected:
    uint32_t crc32Checksum(
        byte* data,
        size_t bytesCount)
    const noexcept;

    virtual UDPEndpoint remoteEndpoint() const = 0;

    void populateQueueWithNewPackets(
        byte* messageData,
        const size_t bytesCount);

    PacketHeader::ChannelIndex nextChannelIndex()
        noexcept;

    void beginPacketsSending();

    virtual string remoteInfo() const = 0;

    virtual LoggerStream errors() const = 0;

    virtual LoggerStream debug() const = 0;

protected:
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


#endif //GEO_NETWORK_CLIENT_OUTGOINGREMOTEBASENODE_H
