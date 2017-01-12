#include "Communicator.h"


Communicator::Communicator(
    as::io_service &ioService,
    const NodeUUID &nodeUUID,
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
        mUUID2AddressService = new UUID2Address(
            ioService,
            uuid2AddressHost,
            uuid2AddressPort);

        mChannelsManager = new ChannelsManager();
        mSocket = new udp::socket(mIOService, udp::endpoint(udp::v4(), nodePort));
        mIncomingMessagesHandler = new IncomingMessagesHandler(mChannelsManager);
        mOutgoingMessagesHandler = new OutgoingMessagesHandler();

    } catch (std::bad_alloc &e) {
        throw MemoryError(
            "Communicator::Communicator: "
                "cant allocate enough memory for one of the Communicator's component.");
    }
}

Communicator::~Communicator() {

    delete mUUID2AddressService;

    delete mChannelsManager;
    delete mSocket;
    delete mOutgoingMessagesHandler;
    delete mIncomingMessagesHandler;
}

void Communicator::beginAcceptMessages() {

    try {
        mUUID2AddressService->registerInGlobalCache(
            mNodeUUID,
            mInterface,
            mPort);

    } catch (std::exception &e) {
        throw RuntimeError(
            "Communicator::beginAcceptMessages: "
                "Can't register in global nodes addresses cache.");
    }

    asyncReceiveData();
}

void Communicator::sendMessage(
    Message::Shared message,
    const NodeUUID &contractorUUID) {

    auto address = mUUID2AddressService->getNodeAddress(contractorUUID);
    ip::udp::endpoint endpoint(
        ip::address::from_string(address.first),
        address.second
    );

    uint16_t channelNumber = mChannelsManager->unusedChannelNumber(endpoint);
    auto *packets = mOutgoingMessagesHandler->processOutgoingMessage(
        message,
        channelNumber
    );

    for (auto const &iter : *packets) {
        sendData(
            iter->packetBytes(),
            address
        );
    }

    packets->clear();
    delete packets;
}

void Communicator::asyncReceiveData() {

    mSocket->async_receive_from(
        boost::asio::buffer(mRecvBuffer),
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
        mIncomingMessagesHandler->processIncomingMessage(
            mRemoteEndpointBuffer,
            mRecvBuffer.data(),
            bytesTransferred
        );

    } else {
        mLog->logError(
            "Communicator::handleReceivedInfo:",
            error.message());
    }

    // In all cases - messages receiving should be continued.
    asyncReceiveData(); // WARNING: stack permanent growing
}

void Communicator::sendData(
    vector<byte> buffer,
    pair<string, uint16_t> address) {

    ip::udp::endpoint destination(
        ip::address::from_string(address.first),
        address.second);

    mSocket->async_send_to(
        as::buffer(
            buffer,
            buffer.size()
        ),
        destination,
        boost::bind(
            &Communicator::handleSend,
            this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred
        )
    );
}

void Communicator::handleSend(
    const boost::system::error_code &error,
    size_t bytesTransferred) {

    if (error) {
        mLog->logError(
            "Communicator::handleSend:",
            error.message()
        );
    } else {
        mLog->logInfo("Communicator::handleSend:",
                      "Packet send " + std::to_string(bytesTransferred));
    }
}
