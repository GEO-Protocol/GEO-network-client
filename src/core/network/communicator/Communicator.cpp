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

    zeroPointers();

    try {
        mSocket = new udp::socket(
            mIOService,
            udp::endpoint(udp::v4(), nodePort)
        );

        mUUID2AddressService = new UUID2Address(
            ioService,
            uuid2AddressHost,
            uuid2AddressPort
        );

        mChannelsManager = new ChannelsManager();

        mIncomingMessagesHandler = new IncomingMessagesHandler(mChannelsManager);

        mOutgoingMessagesHandler = new OutgoingMessagesHandler();

        incomingMessagesSlots = new IncomingMessagesSlots(
            this,
            mLog
        );

    } catch (std::bad_alloc &e) {
        cleanupMemory();
        throw MemoryError("Communicator::Communicator: "
                              "Сant allocate enough memory for one of the Communicator's component.");
    }

    connectIncomingMessagesHanlderSignals();
}

Communicator::~Communicator() {

    cleanupMemory();
}

void Communicator::connectIncomingMessagesHanlderSignals() {

    mIncomingMessagesHandler->messageParsedSignal.connect(
        boost::bind(
            &Communicator::IncomingMessagesSlots::onMessageParsedSlot,
            incomingMessagesSlots
        )
    );
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

    } catch (std::exception &e) {
        throw RuntimeError("Communicator::beginAcceptMessages: "
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

    for (auto const &packet : *packets) {
        sendData(
            packet->packetBytes(),
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
        mLog->logInfo("Communicator::handleReceivedInfo: ",
                       string("Bytes received - ") + to_string(bytesTransferred));

    } else {
        mLog->logError("Communicator::handleReceivedInfo:",
                       error.message());
    }

    // In all cases - messages receiving should be continued.
    // WARNING: stack permanent growing
    asyncReceiveData();
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
        mLog->logError("Communicator::handleSend:",
                       error.message()
        );

    } else {
        mLog->logInfo("Communicator::handleReceivedInfo: ",
                      string("Bytes transaferred - ") + to_string(bytesTransferred));
    }
}

void Communicator::zeroPointers() {

    mSocket = nullptr;
    mUUID2AddressService = nullptr;
    mChannelsManager = nullptr;
    mChannelsManager = nullptr;
    mOutgoingMessagesHandler = nullptr;
    mIncomingMessagesHandler = nullptr;
}

void Communicator::cleanupMemory() {

    if (mSocket != nullptr) {
        delete mSocket;
    }

    if (mUUID2AddressService != nullptr) {
        delete mUUID2AddressService;
    }

    if (mChannelsManager != nullptr) {
        delete mChannelsManager;
    }

    if (mOutgoingMessagesHandler != nullptr) {
        delete mOutgoingMessagesHandler;
    }

    if (mIncomingMessagesHandler != nullptr) {
        delete mIncomingMessagesHandler;
    }
}

Communicator::IncomingMessagesSlots::IncomingMessagesSlots(
    Communicator *communicator,
    Logger *logger) :

    mCommunicator(communicator),
    mLog(logger) {}

void Communicator::IncomingMessagesSlots::onMessageParsedSlot(
    Message::Shared message) {

    mCommunicator->messageReceivedSignal(message);
}
