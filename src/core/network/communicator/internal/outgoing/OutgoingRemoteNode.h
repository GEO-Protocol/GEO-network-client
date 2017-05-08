#ifndef OUTGOINGREMOTENODE_H
#define OUTGOINGREMOTENODE_H

#include "../common/Types.h"

#include "../packet/packet.h"
#include "../uuid2address/UUID2Address.h"
#include "../../../messages/Message.hpp"

#include "../../../../common/NodeUUID.h"
#include "../../../../common/memory/MemoryUtils.h"
#include "../../../../logger/Logger.h"

#include <boost/crc.hpp>
#include <boost/asio/steady_timer.hpp>
#include <queue>



class OutgoingRemoteNode {
public:
    using Shared = shared_ptr<OutgoingRemoteNode>;
    using Unique = unique_ptr<OutgoingRemoteNode>;

public:
    OutgoingRemoteNode(
        const NodeUUID &remoteNodeUUID,
        UUID2Address &uuid2addressService,
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
    const NodeUUID mRemoteNodeUUID;

    UUID2Address &mUUID2AddressService;
    IOService &mIOService;
    UDPSocket &mSocket;
    Logger &mLog;

    queue<pair<byte*, Packet::Size>> mPacketsQueue;
    PacketHeader::ChannelIndex mNextAvailableChannelIndex;
    boost::asio::steady_timer mPacketsSendingTimeoutTimer;
};

#endif // OUTGOINGREMOTENODE_H
