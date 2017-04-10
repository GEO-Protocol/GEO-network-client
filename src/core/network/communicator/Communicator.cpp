#include "Communicator.h"

Communicator::Communicator(
    as::io_service &ioService,
    NodeUUID &nodeUUID,
    const string &nodeInterface,
    const uint16_t nodePort,
    const string &uuid2AddressHost,
    const uint16_t uuid2AddressPort,
    Logger *logger):

    mIOService(ioService),
    mNodeUUID(nodeUUID),
    mInterface(nodeInterface),
    mPort(nodePort),
    mLog(logger){

    try {
        mSocket = unique_ptr<udp::socket>(
            new udp::socket(
                mIOService,
                udp::endpoint(
                    udp::v4(),
                    nodePort
                )
            )
        );

        mUUID2AddressService = unique_ptr<UUID2Address>(
            new UUID2Address(
                ioService,
                uuid2AddressHost,
                uuid2AddressPort
            )
        );

        mChannelsManager = unique_ptr<ChannelsManager>(
            new ChannelsManager(
                mIOService,
                mLog)
        );

        mIncomingMessagesHandler = unique_ptr<IncomingMessagesHandler>(
            new IncomingMessagesHandler(mChannelsManager.get())
        );

        mOutgoingMessagesHandler = unique_ptr<OutgoingMessagesHandler>(
            new OutgoingMessagesHandler()
        );

    } catch (std::bad_alloc &) {
        throw MemoryError("Communicator::Communicator: "
                              "Сan not allocate enough memory for one of the Communicator's component.");
    }

    connectIncomingMessagesHandlerSignals();
}

Communicator::~Communicator() {}

void Communicator::connectIncomingMessagesHandlerSignals() {

    mIncomingMessagesHandler->messageParsedSignal.connect(
        boost::bind(
            &Communicator::onMessageParsedSlot,
            this,
            _1
        )
    );
}

void Communicator::onMessageParsedSlot(
    Message::Shared message) {

    messageReceivedSignal(message);
}

const NodeUUID &Communicator::nodeUUID() const {

    return mNodeUUID;
}

void Communicator::beginAcceptMessages() {

    try {
        mUUID2AddressService->registerInGlobalCache(
            mNodeUUID,
            mInterface,
            mPort
        );

    } catch (std::exception &) {
        throw RuntimeError("Communicator::beginAcceptMessages: "
                               "Can't register in global nodes addresses cache.");
    }
    asyncReceiveData();
}

void Communicator::sendMessage (
    const Message::Shared kMessage,
    const NodeUUID &contractorUUID) {

    auto addressAndPort = mUUID2AddressService->nodeAddressAndPort(contractorUUID);

    // ToDo: at this moment, nodeAddressAndPort() may return "localhost". If so - enpoint would not be created.
    if (addressAndPort.first == "localhost") {
        addressAndPort.first = "127.0.0.1";
    }

    ip::udp::endpoint endpoint(
        ip::address::from_string(addressAndPort.first),
        addressAndPort.second);


    auto numberAndChannel = mChannelsManager->nextOutgoingChannel(endpoint);
    mOutgoingMessagesHandler->processOutgoingMessage(
        kMessage,
        numberAndChannel.first,
        numberAndChannel.second);

    for (auto const &numberAndPacket : *numberAndChannel.second->packets())
        sendData(
            endpoint,
            numberAndPacket.second->packetBytes(),
            numberAndChannel.second);
}

void Communicator::asyncReceiveData() {

    mSocket->async_receive_from(
        boost::asio::buffer(mReceiveBuffer),
        mRemoteEndpointBuffer,
        boost::bind(
            &Communicator::handleReceivedInfo,
            this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred
        )
    );
}

void Communicator::handleReceivedInfo(
    const boost::system::error_code &error,
    size_t bytesTransferred) {

    if (!error || error == boost::asio::error::message_size) {

#ifdef NETWORK_DEBUG_LOG
        {
            auto debug = mLog->debug("Communicator");
            debug << bytesTransferred <<  "B \tRX  [ <= ]";
        }
#endif

        try {
            mIncomingMessagesHandler->processIncomingMessage(
                mRemoteEndpointBuffer,
                mReceiveBuffer.data(),
                bytesTransferred
            );

        } catch (exception &e) {

            mLog->logError("Communicator", e.what());
        }

    } else {
        mLog->logError("Communicator::handleReceivedInfo:",
                       error.message());
    }

    // In all cases - messages receiving should be continued.
    // WARNING: stack permanent growing
    asyncReceiveData();
}

void Communicator::sendData(
    udp::endpoint &endpoint,
    vector<byte> buffer,
    Channel::Shared channel) {

    const auto kBytesSent = mSocket->send_to(
        as::buffer(
            buffer,
            buffer.size()),
        endpoint);

#ifdef NETWORK_DEBUG_LOG
    auto debug = mLog->debug("Communicator");
    debug << kBytesSent <<  "B \tTX  [ => ]";
#endif

    if (channel->canBeRemoved()) {
        mChannelsManager->removeOutgoingChannel(endpoint);
    }
}

void Communicator::handleSend(
    const boost::system::error_code &error,
    size_t bytesTransferred,
    udp::endpoint endpoint,
    Channel::Shared channel) {

    if (channel->canBeRemoved()) {
        mChannelsManager->removeOutgoingChannel(endpoint);
    }

    if (error) {
        mLog->logError("Communicator::handleSend:",
                       error.message()
        );

    } else {

#ifdef NETWORK_DEBUG_LOG
        auto debug = mLog->debug("Communicator");
        debug << bytesTransferred <<  "B \tTX  [ => ]";
#endif

    }
}
