#include "IncomingMessagesHandler.h"

pair<bool, shared_ptr<Message>> MessagesParser::processMessage(
    const byte *messagePart, const size_t receivedBytesCount) {

    // Memory reservation for whole the command
    // to prevent huge amount of memory reallocations.
    mMessageBuffer.reserve(mMessageBuffer.size() + receivedBytesCount);

    // Concatenating with the previously received message parts.
    for (size_t i = 0; i < receivedBytesCount; ++i) {
        mMessageBuffer.push_back(messagePart[i]);
    }

    return tryDeserializeMessage();
}

pair<bool, shared_ptr<Message>> MessagesParser::tryDeserializeMessage() {
    static const size_t MinimalMessageSize =
        sizeof(uint16_t) +      // message size header (2B)
        sizeof(uint32_t) +  // message crc32 header (4B)
        sizeof(uint8_t) +   // type of the message (1B)
        sizeof(uint8_t);    // minimal message content - 1B;

    if (mMessageBuffer.size() < MinimalMessageSize) {
        return make_pair(false, nullptr);
    }

    // Check the message length (2B)
    // Message length includes also the headers
    // (size and crc32)
    uint8_t byte0 = (uint8_t) mMessageBuffer[0];
    uint8_t byte1 = (uint8_t) mMessageBuffer[1];
    uint16_t messageSizeHeader = ((uint16_t) byte1 << 8) | byte0;
    if (mMessageBuffer.size() < messageSizeHeader) {
        // Message received partially.
        return make_pair(false, nullptr);
    }

    // Check message crc (4B)
    // todo: check crc

    // Check message type
    uint8_t messageTypeIDHeader = (uint8_t) mMessageBuffer[6];
    switch (messageTypeIDHeader) {
        case 0:


        default:
            // Unexpected message type received.
            // The message should be rejected.
            mMessageBuffer.resize(0);
            return make_pair(false, nullptr);
    }
}


IncomingMessagesHandler::IncomingMessagesHandler() {

    mChannelsManager = new ChannelsManager();
    mMessagesParser = new MessagesParser();
}

IncomingMessagesHandler::~IncomingMessagesHandler() {

    mParsers.clear();
    delete mChannelsManager;
    delete mMessagesParser;
}

void IncomingMessagesHandler::processIncomingMessage(
    udp::endpoint &clientEndpoint,
    const byte *messagePart,
    const size_t receivedBytesCount) {

    if (receivedBytesCount == 0) {
        return;
    }

    if (messagePart == nullptr) {
        throw ValueError("IncomingMessagesHandler::processIncomingMessage: "
                             "\"messagePart\" can't be null.");
    }


    uint16_t *channelNumber = new(messagePart) uint16_t;

    PacketHeader *packetHeader = new PacketHeader(
        *channelNumber,
        uint16_t(*(new(messagePart + sizeof(uint16_t)) uint16_t)),     // package number
        uint16_t(*(new(messagePart + sizeof(uint16_t) * 2) uint16_t)), // total packets count
        uint16_t(*(new(messagePart + sizeof(uint16_t) * 3) uint16_t))  // bytes count
    );

    auto channel = mChannelsManager->channel(packetHeader->channel());
    channel->addPacket(
        packetHeader->packetNumber(),
        Packet::Shared(new Packet(
            packetHeader,
            messagePart + sizeof(uint16_t) * 4)
        )
    );

    if (channel->checkConsistency()) {
        ConstBytesShared bytes = channel->data();
        mChannelsManager->remove(packetHeader->channel());
    }

    //TODO:: pass bytes to commands parser

    /*auto endpointParser = endpointMessagesParser(clientEndpoint);
    if (endpointParser.first) {
        // Messages parser for received endpoint is already present into the system.
        endpointParser.second.get()->processMessage(
            messagePart,
            receivedBytesCount);

    } else {
        // There is no parser for received endpoint.
        // New one should be created and used.
        auto newEndpointParser = registerNewEndpointParser(clientEndpoint);
        newEndpointParser.get()->processMessage(
            messagePart,
            receivedBytesCount);
    }*/
}

const pair<bool, shared_ptr<MessagesParser>> IncomingMessagesHandler::endpointMessagesParser(
    udp::endpoint &clientEndpoint) const {

    if (mParsers.count(clientEndpoint) > 0) {
        return make_pair(true, mParsers.at(clientEndpoint));

    } else {
        return make_pair(false, nullptr);
    }
}

shared_ptr<MessagesParser> IncomingMessagesHandler::registerNewEndpointParser(
    udp::endpoint &clientEndpoint) {

    if (mParsers.count(clientEndpoint) > 0) {
        // Client endpoint already listed in endpoints map.
        throw ConflictError("IncomingMessagesHandler::registerNewEndpointParser: "
                                "Received endpoint is already listed in the system.");
    }

    // Registering new endpoint.
    auto newEndpointMessagesParser = shared_ptr<MessagesParser>(new MessagesParser());
    mParsers.insert(
        make_pair(
            clientEndpoint,
            newEndpointMessagesParser
        )
    );

    return newEndpointMessagesParser;
}
