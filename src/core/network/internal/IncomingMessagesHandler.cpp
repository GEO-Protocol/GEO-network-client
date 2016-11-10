#include "IncomingMessagesHandler.h"

pair<bool, shared_ptr<Message>> MessagesParser::processMessage(
    const char *messagePart, const size_t receivedBytesCount) {

    // Memory reservation for whole the command
    // to prevent huge amount of memory reallocations.
    mMessageBuffer.reserve(mMessageBuffer.size() + receivedBytesCount);

    // Concatenating with the previously received message parts.
    for (size_t i=0; i<receivedBytesCount; ++i) {
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
        return pair<bool, shared_ptr<Message>>(false, nullptr);
    }

    // Check the message length (2B)
    // Message length includes also the headers
    // (size and crc32)
    uint8_t byte0 = (uint8_t)mMessageBuffer[0];
    uint8_t byte1 = (uint8_t)mMessageBuffer[1];
    uint16_t messageSizeHeader = ((uint16_t)byte1 << 8) | byte0;
    if (mMessageBuffer.size() < messageSizeHeader) {
        // Message received partially.
        return pair<bool, shared_ptr<Message>>(false, nullptr);
    }

    // Check message crc (4B)
    // todo: check crc

    // Check message type
    uint8_t messageTypeIDHeader = (uint8_t)mMessageBuffer[6];
    switch (messageTypeIDHeader) {
        case 0:


        default:
            // Unexpected message type received.
            // The message should be rejected.
            mMessageBuffer.resize(0);
            return pair<bool, shared_ptr<Message>>(false, nullptr);
    }
}


IncomingMessagesHandler::IncomingMessagesHandler() {}

IncomingMessagesHandler::~IncomingMessagesHandler() {
    mParsers.clear();
}

void IncomingMessagesHandler::processIncomingMessage(
    udp::endpoint &clientEndpoint, const char *messagePart, const size_t receivedBytesCount) {

    if (receivedBytesCount == 0) {
        // There is no reason to process the message.
        return;
    }

    if (messagePart == nullptr) {
        throw ValueError("IncomingMessagesHandler::processIncomingMessage: "
                             "\"messagePart\" can't be null.");
    }

    auto endpointParser = endpointMessagesParser(clientEndpoint);
    if (endpointParser.first) {
        // Messages parser for received endpoint is already present into the system.
        endpointParser.second.get()->processMessage(messagePart, receivedBytesCount);

    } else {
        // There is no parser for received endpoint.
        // New one should be created and used.
        auto newEndpointParser = registerNewEndpointParser(clientEndpoint);
        newEndpointParser.get()->processMessage(messagePart, receivedBytesCount);
    }
}

const pair<bool, shared_ptr<MessagesParser>> IncomingMessagesHandler::endpointMessagesParser(
    udp::endpoint &clientEndpoint) const {

    if (mParsers.count(clientEndpoint) > 0) {
        return pair<bool, shared_ptr<MessagesParser>>(true, mParsers.at(clientEndpoint));

    } else {
        return pair<bool, shared_ptr<MessagesParser>>(false, nullptr);
    }
}

shared_ptr<MessagesParser> IncomingMessagesHandler::registerNewEndpointParser(udp::endpoint &clientEndpoint) {
    if (mParsers.count(clientEndpoint) > 0) {
        // Client endpoint already listed in endpoints map.
        throw ConflictError("IncomingMessagesHandler::registerNewEndpointParser: "
                                "Received endpoint is already listed in the system.");
    }

    // Registering new endpoint.
    auto newEndpointMessagesParser = shared_ptr<MessagesParser>(new MessagesParser());
    mParsers.insert(pair<udp::endpoint, shared_ptr<MessagesParser>>(
        clientEndpoint, newEndpointMessagesParser));

    return newEndpointMessagesParser;
}
