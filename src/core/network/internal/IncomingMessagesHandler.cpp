#include "IncomingMessagesHandler.h"

pair<bool, Message::Shared> MessagesParser::processMessage(
    ConstBytesShared messagePart,
    const size_t receivedBytesCount) {

    if (receivedBytesCount < kMininalMessageSize || messagePart.get() == nullptr) {
        return messageInvalidOrIncomplete();
    }

    return tryDeserializeMessage(messagePart);
}

pair<bool, Message::Shared> MessagesParser::tryDeserializeMessage(
    ConstBytesShared messagePart) {

    uint16_t *messageIdentifier = new (const_cast<byte *> (messagePart.get())) uint16_t;
    auto deserializedData = tryDeserializeRequest(
      *messageIdentifier,
      messagePart.get() + sizeof(uint16_t)
    );
    return deserializedData;
}

pair<bool, Message::Shared> MessagesParser::tryDeserializeRequest(
    const uint16_t messageIdentifier,
    const byte *messagePart) {

    switch(messageIdentifier) {

        case Message::MessageTypeID::OpenTrustLineMessageType: {
            Message *message = new AcceptTrustLineMessage(const_cast<byte *>(messagePart));
            return make_pair(
                true,
                Message::Shared(message)
            );
        }

        case Message::MessageTypeID::CloseTrustLineMessageType: {
            Message *message = new RejectTrustLineMessage(const_cast<byte *>(messagePart));
            return make_pair(
                true,
                Message::Shared(message)
            );

        }

        case Message::MessageTypeID::SetTrustLineMessageType: {
            Message *message = new UpdateTrustLineMessage(const_cast<byte *>(messagePart));
            return make_pair(
                true,
                Message::Shared(message)
            );

        }

        default: {
            return tryDeserializeResponse(
                messageIdentifier,
                messagePart
            );
        }
    }

};

pair<bool, Message::Shared> MessagesParser::tryDeserializeResponse(
    const uint16_t messageIdentifier,
    const byte *messagePart) {

    switch(messageIdentifier) {

        case Message::MessageTypeID::ResponseMessageType: {
            Message *message = new Response(const_cast<byte *>(messagePart));
            return make_pair(
                true,
                Message::Shared(message)
            );
        }

        default: {
            return messageInvalidOrIncomplete();
        }
    }
};

pair<bool, Message::Shared> MessagesParser::messageInvalidOrIncomplete() {

    return make_pair(
        false,
        Message::Shared(nullptr)
    );
}


IncomingMessagesHandler::IncomingMessagesHandler(
    ChannelsManager *channelsManager) :

    mChannelsManager(channelsManager) {

    try{
        mMessagesParser = new MessagesParser();

    } catch (std::bad_alloc &e) {
        throw MemoryError("IncomingMessagesHandler::IncomingMessagesHandler: "
                              "Сant allocate enough memory for messages parser.");
    }
}

IncomingMessagesHandler::~IncomingMessagesHandler() {

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
                             "message part can't be null.");
    }

    mPacketsBuffer.reserve(mPacketsBuffer.size() + receivedBytesCount);
    for (size_t i = 0; i < receivedBytesCount; ++i) {
        mPacketsBuffer.push_back(messagePart[i]);
    }

    if(mPacketsBuffer.size() > 1) {
        tryCollectPacket(clientEndpoint);
    }
}

void IncomingMessagesHandler::tryCollectPacket(
    udp::endpoint &clientEndpoint) {

    uint16_t *bytesCount = new (mPacketsBuffer.data()) uint16_t;
    if (mPacketsBuffer.size() >= *bytesCount) {
        uint16_t *channelNumber = new (mPacketsBuffer.data() + Packet::kChannelNumberOffset) uint16_t;
        uint16_t *packageNumber = new (mPacketsBuffer.data() + Packet::kPackageNumberOffset) uint16_t;
        uint16_t *totalPacketsCount = new (mPacketsBuffer.data() + Packet::kTotalPacketsCountOffset) uint16_t;

        PacketHeader *packetHeader = nullptr;
        try {
            packetHeader = new PacketHeader(
                *channelNumber,
                *packageNumber,
                *totalPacketsCount,
                *bytesCount
            );

        } catch (std::bad_alloc &e) {
            throw MemoryError("IncomingMessagesHandler::tryCollectPacket: "
                                  "Can not allocate memory for packet header instance.");
        }

        auto channelAndEndpoint = mChannelsManager->incomingChannel(
            packetHeader->channelNumber(),
            clientEndpoint);

        Packet *packet = nullptr;
        try {
            packet = new Packet(
                packetHeader,
                mPacketsBuffer.data() + Packet::kPacketBodyOffset,
                (size_t) packetHeader->bodyBytesCount()
            );

        } catch (std::bad_alloc &e) {
            throw MemoryError("IncomingMessagesHandler::tryCollectPacket: "
                                  "Can not allocate memory for packet instance.");
        }
        channelAndEndpoint.first->addPacket(
            packetHeader->packetNumber(),
            Packet::Shared(packet)
        );

        cutPacketFromBuffer(*bytesCount);


        if (channelAndEndpoint.first->expectedPacketsCount() == channelAndEndpoint.first->realPacketsCount()) {
            if (channelAndEndpoint.first->checkConsistency()) {
                auto bytesAndCount = channelAndEndpoint.first->data();
                auto resultAndMessage = mMessagesParser->processMessage(
                    bytesAndCount.first,
                    bytesAndCount.second
                );
                if (resultAndMessage.first) {
                    mChannelsManager->removeIncomingChannel(packetHeader->channelNumber());
                    messageParsedSignal(resultAndMessage.second);
                }

            } else {
                mChannelsManager->removeIncomingChannel(packetHeader->channelNumber());
            }
        }
    }
}

void IncomingMessagesHandler::cutPacketFromBuffer(
    size_t bytesCount) {

    mPacketsBuffer.erase(
        mPacketsBuffer.begin(),
        mPacketsBuffer.begin() + bytesCount
    );
}