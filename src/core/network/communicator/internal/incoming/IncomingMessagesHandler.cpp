#include "IncomingMessagesHandler.h"


IncomingMessagesHandler::IncomingMessagesHandler(
    IOService &ioService,
    UDPSocket &socket,
    Logger &logger)
    noexcept:

    mSocket(socket),
    mIOService(ioService),
    mLog(logger),
    mMessagesParser(&logger),
    mRemoteNodesHandler(
        mMessagesParser,
        mLog),
    mCleaningTimer(ioService)
{
#ifdef ENGINE_TYPE_DC
    // Builds Data centers may have signifficantly larger read socket buffer.
    // Large read socket buffer is needed to handle potentially huge amount of messages,
    // that might arrive form the network during max flow calculation.
    const uint32_t kMaxReadSocketSize = 1024*1024*30; // 30MB of data.
#elif

    // Other platforms might be unable to handle extra large buffers,
    // so the default on is used.
    const uint32_t kMaxReadSocketSize = 1024*1024; // 1MB of data
#endif

    boost::asio::socket_base::receive_buffer_size option(kMaxReadSocketSize);
    mSocket.set_option(option);

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
    const boost::system::error_code &errorMessage,
    size_t bytesTransferred)
    noexcept
{
    static auto exponetialTimeoutSeconds = 1;
    static boost::asio::steady_timer waitingTimer(mIOService);


    if (errorMessage) {
        error() << "handleReceivedInfo: ASIO error: " << errorMessage.message();

        // In case of error - wait for some period of time
        // and then restart receiving messages.
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

#ifdef DEBUG_LOG_NETWORK_COMMUNICATOR
            if (mIncomingBuffer.size() > PacketHeader::kSize) {
                const PacketHeader::ChannelIndex kChannelIndex =
                    *(reinterpret_cast<PacketHeader::ChannelIndex*>(
                        mIncomingBuffer.data() + PacketHeader::kChannelIndexOffset));

                const PacketHeader::PacketIndex kPacketIndex =
                    (*(reinterpret_cast<PacketHeader::PacketIndex*>(
                        mIncomingBuffer.data() + PacketHeader::kPacketIndexOffset))) + 1;

                const PacketHeader::TotalPacketsCount kTotalPacketsCount =
                    *(reinterpret_cast<PacketHeader::TotalPacketsCount*>(
                        mIncomingBuffer.data() + PacketHeader::kPacketsCountOffset));

                debug()
                    << setw(4) << bytesTransferred <<  "B RX [ <= ] "
                    << mRemoteEndpointBuffer.address() << ":" << mRemoteEndpointBuffer.port() << "; "
                    << "Channel: " << setw(9) << (kChannelIndex) << "; "
                    << "Packet: " << setw(3) << static_cast<size_t>(kPacketIndex)
                    << "/" << static_cast<size_t>(kTotalPacketsCount);
            }
#endif

            remoteNodeHandler->processIncomingBytesSequence(
                mIncomingBuffer.data(),
                bytesTransferred);

            // Sending all collected messages (if exists) for further processing.
            for (;;) {
                auto message = remoteNodeHandler->popNextMessage();
                if (message != nullptr) {
                    signalMessageParsed(message);
                }
                else {
                    break;
                }
            }
        }

    } catch (exception &e) {
        error() << e.what();
    }


    // In all cases - messages receiving should be continued.
    beginReceivingData();
}

void IncomingMessagesHandler::rescheduleCleaning()
    noexcept
{
    const auto kCleaningInterval = boost::posix_time::seconds(30);

    mCleaningTimer.expires_from_now(kCleaningInterval);
    mCleaningTimer.async_wait([this] (const boost::system::error_code &_) {

#ifdef DEBUG_LOG_NETWORK_COMMUNICATOR_GARBAGE_COLLECTOR
        this->debug() << "Automatic cleaning started";
#endif

        this->mRemoteNodesHandler.removeOutdatedEndpoints();
        this->mRemoteNodesHandler.removeOutdatedChannelsOfPresentEndpoints();

        this->rescheduleCleaning();

#ifdef DEBUG_LOG_NETWORK_COMMUNICATOR_GARBAGE_COLLECTOR
        debug() << "Automatic cleaning finished";
#endif
    });
}

string IncomingMessagesHandler::logHeader()
    noexcept
{
    return "[IncomingMessagesHandler]";
}

LoggerStream IncomingMessagesHandler::info() const
    noexcept
{
    return mLog.info(logHeader());
}

LoggerStream IncomingMessagesHandler::error() const
    noexcept
{
    return mLog.error(logHeader());
}

LoggerStream IncomingMessagesHandler::warning() const
    noexcept
{
    return mLog.warning(logHeader());
}

LoggerStream IncomingMessagesHandler::debug() const
    noexcept
{
#ifdef DEBUG_LOG_NETWORK_COMMUNICATOR
    return mLog.debug(logHeader());
#endif

#ifndef DEBUG_LOG_NETWORK_COMMUNICATOR
    return LoggerStream::dummy();
#endif
}
