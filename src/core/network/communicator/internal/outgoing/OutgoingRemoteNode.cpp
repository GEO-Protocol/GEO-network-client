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
    mNextAvailableChannelIndex(0)
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
    const auto kMessageSizeWithCRC32 = messageBytesCount + sizeof(uint32_t);

    PacketHeader::TotalPacketsCount kTotalPacketsCount =
        static_cast<PacketHeader::TotalPacketsCount>(
            kMessageSizeWithCRC32 / Packet::kMaxSize);

    if (kMessageSizeWithCRC32 % Packet::kMaxSize != 0)
        kTotalPacketsCount += 1;


    PacketHeader::ChannelIndex channelIndex = nextChannelIndex();
    uint32_t crcChecksum = crc32Checksum(
        messageData,
        messageBytesCount);


    size_t processedBytesCount = 0;
    if (kTotalPacketsCount > 1) {
        for (Packet::Index packetIndex=0; packetIndex<kTotalPacketsCount-1; ++packetIndex) {

            // ToDo: remove all previously created packets from the queue to prevent memory leak.
            auto buffer = static_cast<byte*>(malloc(Packet::kMaxSize));

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
                messageData + processedBytesCount,
                Packet::kMaxSize - PacketHeader::kSize);

            processedBytesCount += Packet::kMaxSize;

            const auto packetMaxSize = Packet::kMaxSize;
            mPacketsQueue.push(
                make_pair(
                    buffer,
                    packetMaxSize));
        }
    }

    // Writing last packet
    const PacketHeader::PacketIndex kLastPacketIndex = kTotalPacketsCount - 1;
    const PacketHeader::PacketSize kLastPacketSize =
        static_cast<PacketHeader::PacketSize>(
            kMessageSizeWithCRC32 - processedBytesCount) + PacketHeader::kSize;

    byte *buffer = static_cast<byte*>(malloc(kLastPacketSize));
    if (buffer == nullptr) {
        // ToDo: remove all previously created packets from the queue to prevent memory leak.
        throw bad_alloc();
    }

    memcpy(
        buffer + PacketHeader::kPacketSizeOffset,
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
        &kLastPacketIndex,
        sizeof(kLastPacketIndex));

    memcpy(
        buffer + PacketHeader::kDataOffset,
        messageData + processedBytesCount,
        kLastPacketSize - sizeof(crcChecksum) - PacketHeader::kSize);

    processedBytesCount += kLastPacketSize - sizeof(crcChecksum) - PacketHeader::kSize;

    // Copying CRC32 checksum
    memcpy(
        buffer + PacketHeader::kDataOffset + processedBytesCount,
        &crcChecksum,
        sizeof(crcChecksum));

    processedBytesCount += sizeof(crcChecksum);

    mPacketsQueue.push(
        make_pair(
            buffer,
            kLastPacketSize));
}

void OutgoingRemoteNode::beginPacketsSending()
{
    if (mPacketsQueue.size() == 0) {
        return;
    }


    UDPEndpoint endpoint;
    try {
        endpoint = mUUID2AddressService.endpoint(mRemoteNodeUUID);

    } catch (exception &) {
        errors()
            << "Endpoint can't be fetched from uuid2address. "
            << "No messages can be sent. Outgoing queue cleared.";

        while (mPacketsQueue.size() > 0) {
            const auto packetDataAndSize = mPacketsQueue.front();
            free(packetDataAndSize.first);
            mPacketsQueue.pop();
        }

        return;
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
    return mLog.error(
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
