﻿#include "Communicator.h"

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
            new ChannelsManager(mIOService)
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

void Communicator::sendMessage(
    Message::Shared message,
    const NodeUUID &contractorUUID) {

    auto address = mUUID2AddressService->getNodeAddress(contractorUUID);
    if (address.first == "localhost") {
        address.first = "127.0.0.1";
    }

    ip::udp::endpoint endpoint(
        ip::address::from_string(address.first),
        address.second);


    auto numberAndChannel = mChannelsManager->outgoingChannel(endpoint);

    mOutgoingMessagesHandler->processOutgoingMessage(
        message,
        numberAndChannel.first,
        numberAndChannel.second);

    for (auto const &numberAndPacket : *numberAndChannel.second->packets()) {
        sendData(
            address,
            numberAndPacket.second->packetBytes(),
            numberAndChannel.second);
    }

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
        mLog->logInfo("Communicator::handleReceivedInfo: ",
                      string("Bytes received - ") + to_string(bytesTransferred));
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
    pair<string, uint16_t> address,
    vector<byte> buffer,
    Channel::Shared channel) {

    ip::udp::endpoint destination(
        ip::address::from_string(address.first),
        address.second
    );

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
            boost::asio::placeholders::bytes_transferred,
            destination,
            channel
        )
    );
}

void Communicator::handleSend(
    const boost::system::error_code &error,
    size_t bytesTransferred,
    udp::endpoint endpoint,
    Channel::Shared channel) {

    if (channel->increaseSentPacketsCounter()) {
        mChannelsManager->removeOutgoingChannel(endpoint);
    }

    if (error) {
        mLog->logError("Communicator::handleSend:",
                       error.message()
        );

    } else {
        mLog->logInfo("Communicator::handleSend: ",
                      string("Bytes transferred - ") + to_string(bytesTransferred));
    }
}
