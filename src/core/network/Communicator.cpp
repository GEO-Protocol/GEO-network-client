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

        mSocket = new udp::socket(mIOService, udp::endpoint(udp::v4(), nodePort));
        mIncomingMessagesHandler = new IncomingMessagesHandler();
        mOutgoingMessagesHandler = new OutgoingMessagesHandler();

    } catch (std::exception &) {
        throw MemoryError(
            "Communicator::Communicator: "
                "cant allocate enough memory for one of the Communicator's component.");
    }
}

Communicator::~Communicator() {

    delete mUUID2AddressService;

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
    as::mutable_buffer buffer,
    size_t bufferSize,
    pair<string, uint16_t> address) {

    ip::udp::endpoint destination(
        ip::address::from_string(address.first),
        address.second);

    mSocket->async_send_to(
            as::buffer(
                buffer,
                bufferSize
            ),
            destination,
            boost::bind(
                    &Communicator::handleSentInfo,
                    this,
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred
            )
    );
}

void Communicator::handleSentInfo(const boost::system::error_code &error, size_t bytesTransferred) {

}
