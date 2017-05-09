#include "IncomingChannel.h"


IncomingChannel::IncomingChannel(
    MessagesParser &messagesParser,
    TimePoint &nodeHandlerLastUpdate)
    noexcept :

    mLastRemoteNodeHandlerUpdated(nodeHandlerLastUpdate),
    mMessagesParser(messagesParser),
    mExpectedPacketsCount(0)
{}

IncomingChannel::~IncomingChannel()
    noexcept
{
    clear();
}

/**
 * Reserves memory for expected packets "count";
 * This method is used for optimisation purposes only.
 */
void IncomingChannel::reservePacketsSlots(
    const PacketHeader::TotalPacketsCount count)
    noexcept(false)
{
    // Note:
    // Theoretically, it is possible,
    // that sender node will begin sending several messages into common channel.
    // Each packet contains info about message total packets count.
    // So, theoretically, "count" may change from call to call.
    if (mExpectedPacketsCount != count) {
        mExpectedPacketsCount = count;
        mPackets.reserve(count);
    }
}

/**
 * Inserts packet bytes to the corresponding packet slot of the channel.
 *
 * @param index - specifies packet slot index.
 * @param bytes - bytes sequence of the packet.
 * @param bytesCount - count of bytes in sequence "bytes".
 *
 *
 * @throws bad_alloc;
 */
void IncomingChannel::addPacket(
    const PacketHeader::PacketIndex index,
    byte *bytes,
    const PacketHeader::PacketSize bytesCount)
    noexcept(false)
{
    auto buffer = malloc(bytesCount);
    if (buffer == nullptr) {
        throw bad_alloc();
    }

    // In case if sender node begins sending several message into common channel -
    // packets collision is possible.
    //
    // In case if packet slot is already occupied - no exception should be thrown.
    // Otherwise, both packets would be lost, even if last one potentially may be fully received.
    //
    // To prevent memory leak - previous one packet must be dropped.
    if (mPackets.count(index) > 0){
        free(mPackets[index].first);
    }

    memcpy(
        buffer,
        bytes,
        bytesCount);

    mPackets[index] = make_pair(
        buffer,
        bytesCount);

    mLastUpdated = chrono::steady_clock::now();
    mLastRemoteNodeHandlerUpdated = mLastUpdated;
}

pair<bool, Message::Shared> IncomingChannel::tryCollectMessage()
{
    if (receivedPacketsCount() != expectedPacketsCount()) {
        return make_pair(false, Message::Shared(nullptr));
    }

    size_t totalBytesReceived = 0;
    for (const auto &kPacketIndexAndData : mPackets) {
         totalBytesReceived = kPacketIndexAndData.second.second;
    }

    if (totalBytesReceived <= Packet::kMaxSize - PacketHeader::kSize) {
        // ToDo: optimisation is present:
        // Message consists only one packet.
        // No need for additional copying of it's content into the intermediate buffer.
    }

    // Message consists more than one packet.
    // To be able to deserialize them - all packets must be chained into
    // one memory block.
    auto buffer = tryMalloc(totalBytesReceived);

    size_t currentBufferOffset = 0;
    for (PacketHeader::TotalPacketsCount index=0; index<receivedPacketsCount(); ++index) {
        const auto &kPacketBytesAndBytesCount = mPackets[index];
        memcpy(
            buffer.get() + currentBufferOffset,
            kPacketBytesAndBytesCount.first,
            kPacketBytesAndBytesCount.second);

        currentBufferOffset += kPacketBytesAndBytesCount.second;
    }

    // ToDo: check crc32
    return make_pair(false, Message::Shared(nullptr));

//    return mMessagesParser.processBytesSequence(
//        buffer,
//        totalBytesReceived - Packet::kCRCChecksumBytesCount);
}

/**
 * Removes all packets of the channel and frees the memory.
 */
void IncomingChannel::clear()
    noexcept
{    
    for (const auto &kIndexAndPacketData : mPackets) {
        free(kIndexAndPacketData.second.first);
    }
    mPackets.clear();
}

Packet::Size IncomingChannel::receivedPacketsCount() const
    noexcept
{
    return static_cast<Packet::Size>(mPackets.size());
}

Packet::Size IncomingChannel::expectedPacketsCount() const
    noexcept
{
    return mExpectedPacketsCount;
}

const TimePoint &IncomingChannel::lastUpdated() const
    noexcept
{
    return mLastUpdated;
}
