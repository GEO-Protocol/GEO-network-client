#include "OutgoingRemoteNode.h"


OutgoingRemoteNode::OutgoingRemoteNode(
    const NodeUUID &remoteNodeUUID,
    UUID2Address &uuid2addressService,
    UDPSocket &socket,
    IOService &ioService,
    Logger &logger)
    noexcept :

    mRemoteNodeUUID(remoteNodeUUID),
    mUUID2AddressService(uuid2addressService),
    mIOService(ioService),
    mSocket(socket),
    mLog(logger),
    mNextAvailableChannelIndex(0),
    mCyclesStats(boost::posix_time::microsec_clock::universal_time(), 0),
    mSendingDelayTimer(mIOService)
{}

void OutgoingRemoteNode::sendMessage(
    Message::Shared message)
    noexcept
{
    try {
        // In case if queue already contains packets -
        // then async handler is already scheduled.
        // Otherwise - it must be initialised.
        bool packetsSendingAlreadyScheduled = mPacketsQueue.size() > 0;

        auto bytesAndBytesCount = message->serializeToBytes();
        if (bytesAndBytesCount.second > Message::maxSize()) {
            errors() << "Message is too big to be transferred via the network";
            return;
        }

#ifdef DEBUG_LOG_NETWORK_COMMUNICATOR
        const Message::SerializedType kMessageType =
            *(reinterpret_cast<Message::SerializedType*>(
                bytesAndBytesCount.first.get()));

        debug()
            << "Message of type "
            << static_cast<size_t>(kMessageType)
            << " postponed for the sending";
#endif

        populateQueueWithNewPackets(
            bytesAndBytesCount.first.get(),
            bytesAndBytesCount.second);

        if (not packetsSendingAlreadyScheduled) {
            beginPacketsSending();
        }

    } catch (exception &e) {
        errors()
            << "Exception occured: "
            << e.what();
    }
}

bool OutgoingRemoteNode::containsPacketsInQueue() const
{
    return mPacketsQueue.size() > 0;
}

uint32_t OutgoingRemoteNode::crc32Checksum(
    byte *data,
    size_t bytesCount) const
    noexcept
{
    boost::crc_32_type result;
    result.process_bytes(data, bytesCount);
    return result.checksum();
}

void OutgoingRemoteNode::populateQueueWithNewPackets(
    byte *messageData,
    const size_t messageBytesCount)
{
    const auto kMessageContentWithCRC32BytesCount = messageBytesCount + sizeof(uint32_t);

    PacketHeader::TotalPacketsCount kTotalPacketsCount =
        static_cast<PacketHeader::TotalPacketsCount>(
            kMessageContentWithCRC32BytesCount / Packet::kMaxSize);

    if (kMessageContentWithCRC32BytesCount % Packet::kMaxSize != 0)
        kTotalPacketsCount += 1;


    PacketHeader::ChannelIndex channelIndex = nextChannelIndex();
    uint32_t crcChecksum = crc32Checksum(
        messageData,
        messageBytesCount);


    size_t messageContentBytesProcessed = 0;
    Packet::Index packetIndex = 0;
    if (kTotalPacketsCount > 1) {
        for (; packetIndex<kTotalPacketsCount-1; ++packetIndex) {

            // ToDo: remove all previously created packets from the queue to prevent memory leak.
            auto buffer = static_cast<byte*>(malloc(Packet::kMaxSize));
            if (buffer == nullptr) {
                // Memory error occured.
                // Current packet can't be enqueued, so there is no reason to try to enqueue the rest packets.
                // Already created packets would be removed from the queue on the cleaning stage.
                throw bad_alloc();
            }

            memcpy(
                buffer,
                &Packet::kMaxSize,
                sizeof(Packet::kMaxSize));

            memcpy(
                buffer + PacketHeader::kChannelIndexOffset,
                &channelIndex,
                sizeof(channelIndex));

            memcpy(
                buffer + PacketHeader::kPacketsCountOffset,
                &kTotalPacketsCount,
                sizeof(kTotalPacketsCount));

            memcpy(
                buffer + PacketHeader::kPacketIndexOffset,
                &packetIndex,
                sizeof(packetIndex));

            memcpy(
                buffer + PacketHeader::kDataOffset,
                messageData + messageContentBytesProcessed,
                Packet::kMaxSize - PacketHeader::kSize);

            messageContentBytesProcessed += Packet::kMaxSize - PacketHeader::kSize;

            const auto packetMaxSize = Packet::kMaxSize;
            mPacketsQueue.push(
                make_pair(
                    buffer,
                    packetMaxSize));
        }
    }

    // Writing last packet
    const PacketHeader::PacketSize kLastPacketSize =
        static_cast<PacketHeader::PacketSize>(kMessageContentWithCRC32BytesCount - messageContentBytesProcessed) + PacketHeader::kSize;

    byte *buffer = static_cast<byte*>(malloc(kLastPacketSize));
    if (buffer == nullptr) {
        throw bad_alloc();
    }

    memcpy(
        buffer,
        &kLastPacketSize,
        sizeof(kLastPacketSize));

    memcpy(
        buffer + PacketHeader::kChannelIndexOffset,
        &channelIndex,
        sizeof(channelIndex));

    memcpy(
        buffer + PacketHeader::kPacketsCountOffset,
        &kTotalPacketsCount,
        sizeof(kTotalPacketsCount));

    memcpy(
        buffer + PacketHeader::kPacketIndexOffset,
        &packetIndex,
        sizeof(packetIndex));

    memcpy(
        buffer + PacketHeader::kDataOffset,
        messageData + messageContentBytesProcessed,
        kLastPacketSize - sizeof(crcChecksum) - PacketHeader::kSize);

    messageContentBytesProcessed += kLastPacketSize - sizeof(crcChecksum) - PacketHeader::kSize;

    // Copying CRC32 checksum
    memcpy(
        buffer + PacketHeader::kDataOffset + kLastPacketSize - sizeof(crcChecksum) - PacketHeader::kSize,
        &crcChecksum,
        sizeof(crcChecksum));

    messageContentBytesProcessed += sizeof(crcChecksum);

    mPacketsQueue.push(
        make_pair(
            buffer,
            kLastPacketSize));
}

void OutgoingRemoteNode::beginPacketsSending()
{
    if (mPacketsQueue.empty()) {
        return;
    }


    UDPEndpoint endpoint;
    try {
        endpoint = mUUID2AddressService.endpoint(mRemoteNodeUUID);

    } catch  (exception &) {
        errors()
            << "Endpoint can't be fetched from uuid2address. "
            << "No messages can be sent. Outgoing queue cleared.";

        while (!mPacketsQueue.empty()) {
            const auto packetDataAndSize = mPacketsQueue.front();
            free(packetDataAndSize.first);
            mPacketsQueue.pop();
        }

        return;
    }


    // The next code inserts delay between sending packets in case of high traffic.
    const auto kShortSendingTimeInterval = boost::posix_time::milliseconds(20);
    const auto kTimeoutFromLastSending = boost::posix_time::microsec_clock::universal_time() - mCyclesStats.first;
    if (kTimeoutFromLastSending < kShortSendingTimeInterval) {
        // Increasing short sendings counter.
        mCyclesStats.second += 1;

        const auto kMaxShortSendings = 30;
        if (mCyclesStats.second > kMaxShortSendings) {
            mCyclesStats.second = 0;
            mSendingDelayTimer.expires_from_now(kShortSendingTimeInterval);
            mSendingDelayTimer.async_wait([this] (const boost::system::error_code &_){
                this->beginPacketsSending();
                debug() << "Sending delayed";

            });
            return;
        }

    } else {
        mCyclesStats.second = 0;
    }


    const auto packetDataAndSize = mPacketsQueue.front();
    mSocket.async_send_to(
        boost::asio::buffer(
            packetDataAndSize.first,
            packetDataAndSize.second),
        endpoint,
        [this, endpoint] (const boost::system::error_code &error, const size_t bytesTransferred) {

            const auto packetDataAndSize = mPacketsQueue.front();
            if (bytesTransferred != packetDataAndSize.second) {
                if (error) {
                    errors()
                        << "OutgoingRemoteNode::beginPacketsSending: "
                        << "Next packet can't be sent to the node (" << mRemoteNodeUUID << "). "
                        << "Error code: " << error.value();
                }

                // Removing packet from the memory
                free(packetDataAndSize.first);
                mPacketsQueue.pop();
                if (mPacketsQueue.size() > 0) {
                    beginPacketsSending();
                }

                return;
            }

#ifdef DEBUG_LOG_NETWORK_COMMUNICATOR
            const PacketHeader::ChannelIndex channelIndex =
                *(new(packetDataAndSize.first + PacketHeader::kChannelIndexOffset) PacketHeader::ChannelIndex);

            const PacketHeader::PacketIndex packetIndex =
                *(new(packetDataAndSize.first + PacketHeader::kPacketIndexOffset) PacketHeader::PacketIndex) + 1;

            const PacketHeader::TotalPacketsCount totalPacketsCount =
                *(new(packetDataAndSize.first + PacketHeader::kPacketsCountOffset) PacketHeader::TotalPacketsCount);

            this->debug()
                << setw(4) << bytesTransferred <<  "B TX [ => ] "
                << endpoint.address() << ":" << endpoint.port() << "; "
                << "Channel: " << setw(10) << static_cast<size_t>(channelIndex) << "; "
                << "Packet: " << setw(3) << static_cast<size_t>(packetIndex)
                << "/" << static_cast<size_t>(totalPacketsCount);
#endif

            // Removing packet from the memory
            free(packetDataAndSize.first);
            mPacketsQueue.pop();



            mCyclesStats.first = boost::posix_time::microsec_clock::universal_time();
            if (mPacketsQueue.size() > 0) {
                beginPacketsSending();
            }
        });
}

PacketHeader::ChannelIndex OutgoingRemoteNode::nextChannelIndex()
    noexcept
{
    // Integer overflow is normal here.
    return mNextAvailableChannelIndex++;
}

LoggerStream OutgoingRemoteNode::errors() const
{
    return mLog.warning(
            string("Communicator / OutgoingRemoteNode [")
            + mRemoteNodeUUID.stringUUID()
            + string("]"));
}

LoggerStream OutgoingRemoteNode::debug() const
{
#ifdef DEBUG_LOG_NETWORK_COMMUNICATOR
    return mLog.debug(
        string("Communicator / OutgoingRemoteNode [")
        + mRemoteNodeUUID.stringUUID()
        + string("]"));
#endif

#ifndef DEBUG_LOG_NETWORK_COMMUNICATOR
    return LoggerStream::dummy();
#endif
}
