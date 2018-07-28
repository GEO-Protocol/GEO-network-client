#include "IncomingRemoteNode.h"


IncomingRemoteNode::IncomingRemoteNode(
    const UDPEndpoint &endpoint,
    MessagesParser &messagesParser,
    Logger &logger)
    noexcept:

    mEndpoint(endpoint),
    mMessagesParser(messagesParser),
    mLog(logger)
{}

// ToDo: implement me correct
bool IncomingRemoteNode::isBanned () const
    noexcept
{
    return false;
}

/**
 * @returns shared pointer to the next available message, that was collected from the network.
 * In case, if no messages are availabel - nullptr would be returned.
 *
 * Please, note, that this method may return several messages by several sequential calls.
 * For more details, see comment for the "mCollecteMessages" structure in the header.
 */
Message::Shared IncomingRemoteNode::popNextMessage()
{
    if (mCollectedMessages.empty())
        return Message::Shared(nullptr);

    const auto kMessage = *(prev(mCollectedMessages.cend()));
    mCollectedMessages.pop_back();
    return kMessage;
}

/**
 * Scans all channels of the node, and removes those that are obsolete.
 * Channel is considered obsolete, in case if it doesn't receives any message for some period of time.
 * Please, see method internals for details about max TTL of the channel.
 */
void IncomingRemoteNode::dropOutdatedChannels()
{
    if (mChannels.empty()) {
        return;
    }

    const auto kNow = chrono::steady_clock::now();
    static const auto kMaxTTL = chrono::seconds(5);


    // Forward list is used to not to remove elements of the map while iterating it.
    // All the obsolete channels would be removed at once after scanning;
    forward_list<PacketHeader::ChannelIndex> outdatedChannelsIndexes;
    size_t totalOutdateChannels = 0;

    for (const auto &indexAndChannel : mChannels) {
        if (kNow - indexAndChannel.second->lastUpdated() > kMaxTTL) {
            const auto kChannelIndex = indexAndChannel.first;

#ifdef DEBUG_LOG_NETWORK_COMMUNICATOR
            debug() << "Channel " << kChannelIndex
                    << " of the endpoint " << mEndpoint << " is outdated. Dropped.";
#endif

            outdatedChannelsIndexes.push_front(kChannelIndex);
            ++totalOutdateChannels;
        }
    }

    if (totalOutdateChannels == mChannels.size()) {
        // In case if all channels of the node are outdated -
        // it would much more efficient to clear whole map at once
        // (with only one memory reallocation)
        mChannels.clear();

    } else {
        // Prevent elements reallocation on removing.
        mChannels.reserve(mChannels.size());
        for (const auto kOutdateChannelsIndex : outdatedChannelsIndexes) {
            mChannels.erase(kOutdateChannelsIndex);
        }
        mChannels.rehash(mChannels.size());
    }
}

const UDPEndpoint &IncomingRemoteNode::endpoint() const
    noexcept
{
    return mEndpoint;
}

const TimePoint& IncomingRemoteNode::lastUpdated() const
{
    return mLastUpdated;
}

void IncomingRemoteNode::processIncomingBytesSequence (
    const byte *bytes,
    const size_t count)
    noexcept
{
    if (count == 0) {
        return;
    }

    while (mBuffer.capacity() < (mBuffer.size() + count)) {

#ifdef LINUX
        // Reserve one more memory page (often 4096 B)
        //
        // There is no reason, to reserve less than one memory page:
        // internally reserve will call malloc(),
        // and it would allocate at least one memory page.
        // In case if less than one memory page would be allocated -
        // malloc should be called more often and it would decrease performance.
        mBuffer.reserve(
            mBuffer.capacity() + static_cast<size_t>(getpagesize()));
        continue;
#endif

#ifdef MAC_OS
        // It is expected than one memory page is 4096B.
        // ToDo: find way to get memory page size on the Mac.
        mBuffer.reserve(
            mBuffer.capacity() + 4096);
        continue;
#endif

        // ...
        // Other OS types should go here
        // ...


        // Default reservation
        mBuffer.reserve(
            mBuffer.capacity() + 1024);
    }


    // Appending received data to the previously received
    mBuffer.resize(mBuffer.size() + count);
    memcpy(&mBuffer[mBuffer.size() - count], bytes, count * sizeof(byte));

    size_t totalMessagesCollected = 0;
    while (true) {
        bool shouldTryCollectOneMorePacket = tryCollectNextPacket();
        if (not shouldTryCollectOneMorePacket) {
            break;
        }

        ++totalMessagesCollected;
    }

    if (totalMessagesCollected > 0) {
        // Free the unused buffer space.
        mBuffer.shrink_to_fit();
    }
}

/*
 * Returns "true" if packet was collected and one more collect attempt may be done.
 * Otherwise - returns "false";
 */
bool IncomingRemoteNode::tryCollectNextPacket ()
{
    if (mBuffer.size() < Packet::kMinSize) {
        return false;
    }

    // Header parsing
    const PacketHeader::PacketSize kHeaderAndBodyBytesCount =
        *(reinterpret_cast<PacketHeader::PacketSize*>(
            mBuffer.data()));

    const PacketHeader::ChannelIndex kChannelIndex =
        *(reinterpret_cast<PacketHeader::ChannelIndex*>(
            mBuffer.data() + PacketHeader::kChannelIndexOffset));

    const PacketHeader::PacketIndex kPacketIndex =
        *(reinterpret_cast<PacketHeader::PacketIndex*>(
            mBuffer.data() + PacketHeader::kPacketIndexOffset));

    const PacketHeader::TotalPacketsCount kTotalPacketsCount =
        *(reinterpret_cast<PacketHeader::TotalPacketsCount*>(
            mBuffer.data() + PacketHeader::kPacketsCountOffset));

    debug()
        << "Packet received. Remote endpoint - " << mEndpoint
        << "; Channel index - " << int(kChannelIndex)
        << "; Packet index - " << int(kPacketIndex)
        << "; Total packets count in message - " << int(kTotalPacketsCount);

    if (kHeaderAndBodyBytesCount < Packet::kMinSize
        || kHeaderAndBodyBytesCount > Packet::kMaxSize) {

        // Packet bytes count field can't be less than minimal header size,
        // and can't be greater than max packet size.
        //
        // In case when this field is invalid - it is impossible to know how many bytes are in current packet,
        // and where is the first byte of next one packet. The whole bytes sequence must be dropped,
        // and the packets exchange must be started again.
        //
        // To prevent possible vulnerability with infinite random bytes sequence (dos) -
        // all the network data from this endpoint must be rejected for some period of time.
        // This period of time must be exponentially increased if this error will not be eliminated
        // on the next round.

        dropEntireIncomingFlow();

        // ToDo: ban the node.
        return false;
    }

    if (kTotalPacketsCount == 0
        || kPacketIndex > kTotalPacketsCount) {

        // Invalid bytes flow occurred.
        dropEntireIncomingFlow();

        // ToDo: ban the node.
        return false;
    }

    if (mBuffer.size() < kHeaderAndBodyBytesCount) {
        // There is no sufficient data was received yet.
        // Can't start collecting packet.
        return false;
    }

    auto channel = findChannel(kChannelIndex);
    channel->reservePacketsSlots(kTotalPacketsCount);
    channel->addPacket(
        kPacketIndex,
        mBuffer.data() + PacketHeader::kSize,
        kHeaderAndBodyBytesCount - PacketHeader::kSize);

    // Cut bytes transferred to the packet from the buffer
    mBuffer.erase(
        mBuffer.cbegin(),
        mBuffer.cbegin() + kHeaderAndBodyBytesCount);

    const auto kFlagAndMessage = channel->tryCollectMessage();
    if (kFlagAndMessage.first) {
        debug() << "Collected message of type " << kFlagAndMessage.second->typeID();
        mCollectedMessages.push_back(kFlagAndMessage.second);
        mChannels.erase(kChannelIndex);
    }

    // Return true if one more potential packet is present in buffer.
    return mBuffer.size() >= Packet::kMinSize;
}

void IncomingRemoteNode::dropEntireIncomingFlow()
{
    // Note:
    // Corresponding channel must not be cleared. The packet may be broken by the network, and not obfuscated.
    // Current message would not collect and would be dropped anyway, but the packet may pointing to the channel that not belongs it.
    // (in case if channel number was also modified).
    // In case if the channel at which the packet points would be removed - other message may be broken.

    mBuffer.clear();
    mBuffer.shrink_to_fit();
}

IncomingChannel* IncomingRemoteNode::findChannel(
    const PacketHeader::ChannelIndex index)
{
    if (mChannels.count(index) == 0)
        mChannels.emplace(
            index,
            make_unique<IncomingChannel>(
                mMessagesParser,
                mLastUpdated,
                mLog));

    return mChannels[index].get();
}

LoggerStream IncomingRemoteNode::debug() const
    noexcept
{
#ifdef DEBUG_LOG_NETWORK_COMMUNICATOR
    return mLog.debug("Communicator / IncomingRemoteNode");
#endif

#ifndef DEBUG_LOG_NETWORK_COMMUNICATOR
    return LoggerStream::dummy();
#endif
}
