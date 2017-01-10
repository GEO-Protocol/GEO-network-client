#include "IncomingMessagesHandler.h"

pair<bool, Message::Shared> MessagesParser::processMessage(
    const byte *messagePart,
    const size_t receivedBytesCount) {

    if (receivedBytesCount < kMininalMessageSize || messagePart == nullptr) {
        return messageInvalidOrIncomplete();
    }

    return tryDeserializeMessage(messagePart);
}

pair<bool, Message::Shared> MessagesParser::tryDeserializeMessage(
    const byte *messagePart) {

    uint16_t *messageIdentifier = new (const_cast<byte *> (messagePart)) uint16_t;
    switch(*messageIdentifier) {

        case Message::MessageTypeID::ProcessingReportMessage: {

            return make_pair(
                true, Message::Shared(nullptr));
        }

        default: {

            return messageInvalidOrIncomplete();
        }
    }
}

pair<bool, Message::Shared> MessagesParser::messageInvalidOrIncomplete() {

    return make_pair(
        false,
        Message::Shared(nullptr));
}


IncomingMessagesHandler::IncomingMessagesHandler() {

    mChannelsManager = new ChannelsManager();
    mMessagesParser = new MessagesParser();
}

IncomingMessagesHandler::~IncomingMessagesHandler() {

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

    mPacketsBuffer.reserve(mPacketsBuffer.size() + receivedBytesCount);
    for (size_t i = 0; i < receivedBytesCount; ++i) {
        mPacketsBuffer.push_back(messagePart[i]);
    }

    if(mPacketsBuffer.size() > 1) {
        tryCollectPacket();
    }
}

void IncomingMessagesHandler::tryCollectPacket() {

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

        auto channel = mChannelsManager->channel(packetHeader->channelNumber());
        Packet *packet = nullptr;
        try {
            packet = new Packet(
                packetHeader,
                mPacketsBuffer.data() + Packet::kPacketBodyOffset,
                (size_t) packetHeader->bodyBytesCount());

        } catch (std::bad_alloc &e) {
            throw MemoryError("IncomingMessagesHandler::tryCollectPacket: "
                                  "Can not allocate memory for packet instance.");
        }
        channel->addPacket(
            packetHeader->packetNumber(),
            Packet::Shared(packet)
        );

        cutPacketFromBuffer(*bytesCount);


        if (channel->expectedPacketsCount() == channel->realPacketsCount()) {
            if (channel->checkConsistency()) {
                auto data = channel->data();
                mMessagesParser->processMessage(
                  data.second.get(),
                  data.first
                );
                mChannelsManager->remove(packetHeader->channelNumber());

            } else {
                mChannelsManager->remove(packetHeader->channelNumber());
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

