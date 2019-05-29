#include "OutgoingRemoteBaseNode.h"

OutgoingRemoteBaseNode::OutgoingRemoteBaseNode(
    UDPSocket &socket,
    IOService &ioService,
    Logger &logger):

    mIOService(ioService),
    mSocket(socket),
    mLog(logger),
    mNextAvailableChannelIndex(0),
    mCyclesStats(boost::posix_time::microsec_clock::universal_time(), 0),
    mSendingDelayTimer(mIOService)
{}

OutgoingRemoteBaseNode::~OutgoingRemoteBaseNode() {}

PacketHeader::ChannelIndex OutgoingRemoteBaseNode::nextChannelIndex()
    noexcept
{
    // Integer overflow is normal here.
    return mNextAvailableChannelIndex++;
}

void OutgoingRemoteBaseNode::sendMessage(
    Message::Shared message)
noexcept
{
    try {
        // In case if queue already contains packets -
        // then async handler is already scheduled.
        // Otherwise - it must be initialised.
        bool packetsSendingAlreadyScheduled = !mPacketsQueue.empty();

        auto bytesAndBytesCount = preprocessMessage(message);
        if (bytesAndBytesCount.second > Message::maxSize()) {
            errors() << "Message is too big to be transferred via the network";
            return;
        }

#ifdef DEBUG_LOG_NETWORK_COMMUNICATOR
        debug()
            << "Message of type " << message->typeID() << " encrypted("
            << message->isEncrypted() <<") postponed for the sending";
#endif

        populateQueueWithNewPackets(
            bytesAndBytesCount.first.get(),
            bytesAndBytesCount.second);

        if (not packetsSendingAlreadyScheduled) {
            beginPacketsSending();
        }

    } catch (exception &e) {
        errors()
            << "Exception occurred: "
            << e.what();
    }
}

bool OutgoingRemoteBaseNode::containsPacketsInQueue() const
{
    return !mPacketsQueue.empty();
}

uint32_t OutgoingRemoteBaseNode::crc32Checksum(
    byte *data,
    size_t bytesCount) const
    noexcept
{
    boost::crc_32_type result;
    result.process_bytes(data, bytesCount);
    return result.checksum();
}

void OutgoingRemoteBaseNode::populateQueueWithNewPackets(
    byte *messageData,
    const size_t messageBytesCount)
{
    const auto kMessageContentWithCRC32BytesCount = messageBytesCount + sizeof(uint32_t);

    PacketHeader::TotalPacketsCount kTotalPacketsCount = 0;
    for(size_t bytesLeft=kMessageContentWithCRC32BytesCount; bytesLeft; ++kTotalPacketsCount) {
        bytesLeft -= std::min(bytesLeft, Packet::kMaxSize - PacketHeader::kSize);
    }

    PacketHeader::ChannelIndex channelIndex = nextChannelIndex();
    uint32_t crcChecksum = crc32Checksum(
        messageData,
        messageBytesCount);

    size_t messageContentBytesProcessed = 0;
    size_t messageCrc32ChecksumBytesProcessed = 0;
    Packet::Index packetIndex = 0;
    if (kTotalPacketsCount > 1) {
        for (; packetIndex<kTotalPacketsCount-1; ++packetIndex) {

            // ToDo: remove all previously created packets from the queue to prevent memory leak.
            auto buffer = static_cast<byte*>(malloc(Packet::kMaxSize));
            if (buffer == nullptr) {
                // Memory error occurred.
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

            size_t usefulBytesCount = Packet::kMaxSize - PacketHeader::kSize;
            size_t messageLeftover = std::min(usefulBytesCount,
                                              messageBytesCount - messageContentBytesProcessed);
            memcpy(
                buffer + PacketHeader::kDataOffset,
                messageData + messageContentBytesProcessed,
                messageLeftover);

            messageContentBytesProcessed += messageLeftover;

            size_t bytesLeftover = usefulBytesCount - messageLeftover;
            size_t checksumPartial = std::min(bytesLeftover, (size_t)sizeof(crcChecksum));
            memcpy(
                buffer + PacketHeader::kDataOffset + messageLeftover,
                &crcChecksum,
                checksumPartial);
            messageCrc32ChecksumBytesProcessed += checksumPartial;
            messageContentBytesProcessed += checksumPartial;

            const auto packetMaxSize = Packet::kMaxSize;
            mPacketsQueue.push(
                make_pair(
                    buffer,
                    packetMaxSize));
        }
    }

    // Writing last packet
    const PacketHeader::PacketSize kLastPacketSize =
        static_cast<PacketHeader::PacketSize>(kMessageContentWithCRC32BytesCount -
            messageContentBytesProcessed) + PacketHeader::kSize;

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

    size_t usefulBytesCount = Packet::kMaxSize - PacketHeader::kSize;
    size_t messageLeftover = std::min(usefulBytesCount,
                                      messageBytesCount - std::min(messageBytesCount, messageContentBytesProcessed));
    memcpy(
        buffer + PacketHeader::kDataOffset,
        messageData + messageContentBytesProcessed,
        messageLeftover);

    messageContentBytesProcessed += messageLeftover;

    // Copying CRC32 checksum
    auto checksumLeftover = (size_t)(sizeof(crcChecksum) - messageCrc32ChecksumBytesProcessed);
    memcpy(
        buffer + PacketHeader::kDataOffset + messageLeftover,
        (uint8_t *)&crcChecksum + messageCrc32ChecksumBytesProcessed,
        checksumLeftover);

    mPacketsQueue.push(
        make_pair(
            buffer,
            kLastPacketSize));
}

void OutgoingRemoteBaseNode::beginPacketsSending()
{
    if (mPacketsQueue.empty()) {
        return;
    }

    UDPEndpoint endpoint;
    try {
        endpoint = remoteEndpoint();
        debug() << "Endpoint address " << endpoint.address().to_string();
        debug() << "Endpoint port " << endpoint.port();
    } catch  (exception &) {
        errors()
            << "Endpoint can't be fetched from Contractor. "
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
                    errors() << "beginPacketsSending: "
                             << "Next packet can't be sent to the node (" << remoteInfo() << "). "
                             << "Error code: " << error.value();
                }

                // Removing packet from the memory
                free(packetDataAndSize.first);
                mPacketsQueue.pop();
                if (!mPacketsQueue.empty()) {
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

            this->debug() << setw(4) << bytesTransferred <<  "B TX [ => ] "
                          << endpoint.address() << ":" << endpoint.port() << "; "
                          << "Channel: " << setw(10) << static_cast<size_t>(channelIndex) << "; "
                          << "Packet: " << setw(3) << static_cast<size_t>(packetIndex)
                          << "/" << static_cast<size_t>(totalPacketsCount);
#endif

            // Removing packet from the memory
            free(packetDataAndSize.first);
            mPacketsQueue.pop();

            mCyclesStats.first = boost::posix_time::microsec_clock::universal_time();
            if (!mPacketsQueue.empty()) {
                beginPacketsSending();
            }
        });
}
