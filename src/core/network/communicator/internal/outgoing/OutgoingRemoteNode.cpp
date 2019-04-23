#include "OutgoingRemoteNode.h"

OutgoingRemoteNode::OutgoingRemoteNode(
    Contractor::Shared remoteContractor,
    UDPSocket &socket,
    IOService &ioService,
    ContractorsManager *contractorsManager,
    Logger &logger)
    noexcept :

    mRemoteContractor(remoteContractor),
    OutgoingRemoteBaseNode(
        socket,
        ioService,
        contractorsManager,
        logger
    )
{}

void OutgoingRemoteNode::beginPacketsSending()
{
    if (mPacketsQueue.empty()) {
        return;
    }

    UDPEndpoint endpoint;
    try {
        endpoint = as::ip::udp::endpoint(
            as::ip::address_v4::from_string(mRemoteContractor->mainAddress()->host()),
            mRemoteContractor->mainAddress()->port());
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
                    errors()
                        << "OutgoingRemoteNode::beginPacketsSending: "
                        << "Next packet can't be sent to the node (" << mRemoteContractor->getID() << "). "
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
            if (!mPacketsQueue.empty()) {
                beginPacketsSending();
            }
        });
}

LoggerStream OutgoingRemoteNode::errors() const
{
    return mLog.warning(
        string("Communicator / OutgoingRemoteNode [")
        + to_string(mRemoteContractor->getID())
        + string("]"));
}

LoggerStream OutgoingRemoteNode::debug() const
{
#ifdef DEBUG_LOG_NETWORK_COMMUNICATOR
    return mLog.debug(
        string("Communicator / OutgoingRemoteNodeNew [")
        + to_string(mRemoteContractor->getID())
        + string("]"));
#endif

#ifndef DEBUG_LOG_NETWORK_COMMUNICATOR
    return LoggerStream::dummy();
#endif
}
