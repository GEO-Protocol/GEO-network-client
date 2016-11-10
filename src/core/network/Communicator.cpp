#include "Communicator.h"

Communicator::Communicator(
    as::io_service &ioService, const string &interface, const uint16_t port):
    mIOService(ioService) {

    mSocket = new udp::socket(mIOService, udp::endpoint(udp::v4(), port));
    mIncomingMessagesHandler = new IncomingMessagesHandler();
    mOutgoingMessagesHandler = new OutgoingMessagesHandler();
}

Communicator::~Communicator() {
    delete mSocket;
    delete mOutgoingMessagesHandler;
    delete mIncomingMessagesHandler;
}

void Communicator::beginAcceptMessages() {
    // ...
    // all preparing things goes here

    asyncReceiveData();
}

void Communicator::asyncReceiveData() {
    mSocket->async_receive_from(
        boost::asio::buffer(mRecvBuffer), mRemoteEndpointBuffer,
        boost::bind(
            &Communicator::handleReceivedInfo, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
}

void Communicator::handleReceivedInfo(const boost::system::error_code &error, size_t bytesTransferred) {
    if (!error || error == boost::asio::error::message_size) {
        mIncomingMessagesHandler->processIncomingMessage(
            mRemoteEndpointBuffer, mRecvBuffer.data(), bytesTransferred);
    }

    // In all cases - messages receiving should be continued.
    asyncReceiveData(); // WARNING: stack permanent growing
}
