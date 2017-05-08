#include "IncomingChannel.h"


/**
 * @brief IncomingChannel::reservePacketsSlots
 *
 * Each packet contains information about total count of packets of the message.
 * Thus, this method may be called right after first packet received to speedup packets collecting.
 * It used only in optimasation approaches.
 */
IncomingChannel::IncomingChannel(
    MessagesParser &messagesParser,
    TimePoint &nodeHandlerLastUpdate)
    noexcept :

    mExpectedPacketsCount(0),
    mLastRemoteNodeHandlerUpdated(nodeHandlerLastUpdate),
    mMessagesParser(messagesParser)
{}

IncomingChannel::~IncomingChannel()
{
    clear();
}


void IncomingChannel::reserveSlots(
    const PacketHeader::TotalPacketsCount count)
{
    if (mExpectedPacketsCount < count) {
        mExpectedPacketsCount = count;
        mPackets.reserve(count);
    }
}

void IncomingChannel::addPacket(
    const PacketHeader::PacketIndex index,
    byte *bytes,
    const PacketHeader::PacketSize bytesCount)
{
    void* buffer = malloc(bytesCount);
    if (buffer == nullptr) {
        throw bad_alloc();
    }

    // remove previous packet
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
    if (receivedPacketsCount() != expectedPacketsCount())
        return make_pair(false, Message::Shared(nullptr));

    return make_pair(false, Message::Shared(nullptr));

//    // Collecting buffer from several packets
//    size_t totalBytesCount = 0;
//    for (PacketHeader::PacketIndex index=0; index<receivedPacketsCount(); ++index)
//        totalBytesCount += mPackets[index].second;

//    // Copying all the packets into one buffer.
//    size_t currentBufferOffset = 0;
//    auto buffer = tryMalloc(totalBytesCount);

//    for (PacketHeader::PacketIndex index=0; index<receivedPacketsCount(); ++index) {
//        const auto kPacketBytesAndBytesCount = mPackets[index];
//        memcpy(
//            buffer.get() + currentBufferOffset,
//            kPacketBytesAndBytesCount.first,
//            kPacketBytesAndBytesCount.second);

//        currentBufferOffset += kPacketBytesAndBytesCount.second;
//    }

//    // ToDo: check crc32

//    return mMessagesParser.processBytesSequence(
//        buffer,
//        totalBytesCount-sizeof(uint32_t));
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
{
    return static_cast<Packet::Size>(mPackets.size());
}

Packet::Size IncomingChannel::expectedPacketsCount() const
{
    return mExpectedPacketsCount;
}

const TimePoint &IncomingChannel::lastUpdated() const
{
    return mLastUpdated;
}
