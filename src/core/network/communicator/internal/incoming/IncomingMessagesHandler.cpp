#include "IncomingMessagesHandler.h"


IncomingMessagesHandler::IncomingMessagesHandler(
    IOService &ioService,
    UDPSocket &socket,
    Logger &logger)
    noexcept:

    mSocket(socket),
    mIOService(ioService),
    mLog(logger),
    mMessagesParser(logger),
    mRemoteNodesHandler(
        mMessagesParser,
        mLog),
    mCleaningTimer(ioService)
{
    rescheduleCleaning();
}

void IncomingMessagesHandler::beginReceivingData ()
    noexcept
{
    mSocket.async_receive_from(
       boost::asio::buffer(mIncomingBuffer),
       mRemoteEndpointBuffer,
       boost::bind(
           &IncomingMessagesHandler::handleReceivedInfo,
           this,
           boost::asio::placeholders::error,
           boost::asio::placeholders::bytes_transferred));
}

void IncomingMessagesHandler::handleReceivedInfo(
    const boost::system::error_code &error,
    size_t bytesTransferred)
    noexcept
{
    static auto exponetialTimeoutSeconds = 1;
    static boost::asio::steady_timer waitingTimer(mIOService);


    if (error) {
        mLog.error("IncomingMessagesHandler::handleReceivedInfo")
            << "ASIO error: "
            << error.message();

        // In case of error - wait for some period of time
        // and then restart receiveing messages.
        exponetialTimeoutSeconds = exponetialTimeoutSeconds * 2;
        waitingTimer.expires_from_now(
            chrono::seconds(
                exponetialTimeoutSeconds));

        waitingTimer.async_wait([this] (const boost::system::error_code&) {
            beginReceivingData();});

        return;
    }


    try {
        auto remoteNodeHandler = mRemoteNodesHandler.handler(mRemoteEndpointBuffer);
        if (remoteNodeHandler->isBanned()) {
            info() << bytesTransferred <<  "B \tRX  [ <= ] from "
                   << mRemoteEndpointBuffer.address().to_string()
                   << ". IGNORED!";

            beginReceivingData();
            return;

        } else {

#ifdef NETWORK_DEBUG_LOG
            if (mIncomingBuffer.size() > PacketHeader::kSize) {
                const PacketHeader::ChannelIndex channelIndex = *(new(mIncomingBuffer.data() + PacketHeader::kChannelIndexOffset) PacketHeader::ChannelIndex);
                const PacketHeader::PacketIndex packetIndex = *(new(mIncomingBuffer.data() + PacketHeader::kPacketIndexOffset) PacketHeader::ChannelIndex) + 1;
                const PacketHeader::TotalPacketsCount totalPacketsCount = *(new(mIncomingBuffer.data() + PacketHeader::kPacketsCountOffset) PacketHeader::TotalPacketsCount);

                this->debug()
                    << setw(4) << bytesTransferred <<  "B RX [ <= ] "
                    << mRemoteEndpointBuffer.address() << ":" << mRemoteEndpointBuffer.port() << "; "
                    << "Channel: " << setw(6) << static_cast<int>(channelIndex) << "; "
                    << "Packet: " << setw(6) << static_cast<int>(packetIndex) << "/" << totalPacketsCount;
            }
#endif

            remoteNodeHandler->processIncomingBytesSequence(
                mIncomingBuffer.data(),
                bytesTransferred);

            // Sending all collected messages (if exists) for further processing.
            for (;;) {
                auto message = remoteNodeHandler->popNextMessage();
                if (message != nullptr)
                    signalMessageParsed(message);
                else
                    break;
            }
        }

    } catch (exception &e) {
        errors() << e.what();
    }


    // In all cases - messages receiving should be continued.
    // WARNING: stack permanent growing
    beginReceivingData();
}

void IncomingMessagesHandler::rescheduleCleaning()
    noexcept
{
    const auto kCleaningInterval = boost::posix_time::seconds(10);

    mCleaningTimer.expires_from_now(kCleaningInterval);
    mCleaningTimer.async_wait([this] (const boost::system::error_code &_) {

#ifdef NETWORK_DEBUG_LOG
        this->debug() << "Automatic cleaning started";
#endif // NETWORK_DEBUG_LOG

        this->mRemoteNodesHandler.removeOutdatedEndpoints();
        this->mRemoteNodesHandler.removeOutdatedChannelsOfPresentEndpoints();

        this->rescheduleCleaning();

#ifdef NETWORK_DEBUG_LOG
        this->debug() << "Automatic cleaning finished";
#endif // NETWORK_DEBUG_LOG
    });
}

LoggerStream IncomingMessagesHandler::info() const
    noexcept
{
    return mLog.info("IncomingMessagesHandler");
}

LoggerStream IncomingMessagesHandler::errors() const
    noexcept
{
    return mLog.error("IncomingMessagesHandler");
}

LoggerStream IncomingMessagesHandler::debug() const
    noexcept
{
#ifdef NETWORK_DEBUG_LOG
    return mLog.debug("IncomingMessagesHandler");
#endif

#ifndef NETWORK_DEBUG_LOG
    return LoggerStream();
#endif
}
