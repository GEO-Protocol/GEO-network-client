#include "IncomingMessagesHandler.h"

pair<bool, Message::Shared> MessagesParser::processMessage(
    BytesShared messagePart,
    const size_t receivedBytesCount) {

    if (receivedBytesCount < kMinimalMessageSize || messagePart.get() == nullptr) {
        return messageInvalidOrIncomplete();
    }

    return tryDeserializeMessage(messagePart);
}

pair<bool, Message::Shared> MessagesParser::tryDeserializeMessage(
    BytesShared messagePart) {

    try {
        uint16_t *messageIdentifier = new (messagePart.get()) uint16_t;
        auto deserializedData = tryDeserializeRequest(
            *messageIdentifier,
            messagePart
        );
        return deserializedData;

    } catch (...) {
        return messageInvalidOrIncomplete();
    }
}

pair<bool, Message::Shared> MessagesParser::tryDeserializeRequest(
    const uint16_t messageIdentifier,
    BytesShared messagePart) {

    switch(messageIdentifier) {

        case Message::OpenTrustLineMessageType: {
            return make_pair(
                true,
                static_pointer_cast<Message>(
                    make_shared<AcceptTrustLineMessage>(messagePart)
                )
            );
        }

        case Message::CloseTrustLineMessageType: {
            return make_pair(
                true,
                static_pointer_cast<Message>(
                    make_shared<RejectTrustLineMessage>(messagePart)
                )
            );
        }

        case Message::SetTrustLineMessageType: {
            return make_pair(
                true,
                static_pointer_cast<Message>(
                    make_shared<UpdateTrustLineMessage>(messagePart)
                )
            );
        }

        case Message::FirstLevelRoutingTableOutgoingMessageType: {
            return make_pair(
                true,
                static_pointer_cast<Message>(
                    make_shared<FirstLevelRoutingTableIncomingMessage>(messagePart)
                )
            );
        }

        case Message::SecondLevelRoutingTableOutgoingMessageType: {
            return make_pair(
                true,
                static_pointer_cast<Message>(
                    make_shared<SecondLevelRoutingTableIncomingMessage>(messagePart)
                )
            );
        }

        case Message::Payments_ReceiverInitPayment: {
            return make_pair(
                true,
                static_pointer_cast<Message>(
                    make_shared<ReceiverInitPaymentMessage>(messagePart)
                )
            );
        }
        case Message::Payments_ReceiverApprove: {
            return make_pair(
                true,
                static_pointer_cast<Message>(
                    make_shared<ReceiverApproveMessage>(messagePart)
                )
            );
        }

        case Message::InBetweenNodeTopologyMessage: {
            return make_pair(
                true,
                static_pointer_cast<Message>(
                    make_shared<InBetweenNodeTopologyMessage>(messagePart)
                )
            );
        }

        case Message::BoundaryNodeTopologyMessage: {
            return make_pair(
                true,
                static_pointer_cast<Message>(
                    make_shared<BoundaryNodeTopologyMessage>(messagePart)
                )
            );
        }

        case Message::InitiateMaxFlowCalculationMessageType: {
            return make_pair(
                true,
                static_pointer_cast<Message>(
                    make_shared<InitiateMaxFlowCalculationMessage>(messagePart)
                )
            );
        }

        case Message::ResultMaxFlowCalculationMessageType: {
            return make_pair(
                true,
                static_pointer_cast<Message>(
                    make_shared<ResultMaxFlowCalculationMessage>(messagePart)
                )
            );
        }

        case Message::MaxFlowCalculationSourceFstLevelMessageType: {
            return make_pair(
                true,
                static_pointer_cast<Message>(
                    make_shared<MaxFlowCalculationSourceFstLevelMessage>(messagePart)
                )
            );
        }

        case Message::MaxFlowCalculationTargetFstLevelMessageType: {
            return make_pair(
                true,
                static_pointer_cast<Message>(
                    make_shared<MaxFlowCalculationTargetFstLevelMessage>(messagePart)
                )
            );
        }

        case Message::MaxFlowCalculationSourceSndLevelMessageType: {
            return make_pair(
                true,
                static_pointer_cast<Message>(
                    make_shared<MaxFlowCalculationSourceSndLevelMessage>(messagePart)
                )
            );
        }

        case Message::MaxFlowCalculationTargetSndLevelMessageType: {
            return make_pair(
                true,
                static_pointer_cast<Message>(
                    make_shared<MaxFlowCalculationTargetSndLevelMessage>(messagePart)
                )
            );
        }

        default: {
            return tryDeserializeResponse(
                messageIdentifier,
                messagePart
            );
        }

    }
}

pair<bool, Message::Shared> MessagesParser::tryDeserializeResponse(
    const uint16_t messageIdentifier,
    BytesShared messagePart) {

    switch(messageIdentifier) {

        case Message::MessageTypeID::ResponseMessageType: {
            return make_pair(
                true,
                static_pointer_cast<Message>(
                    make_shared<Response>(messagePart)
                )
            );
        }

        case Message::MessageTypeID::RoutingTablesResponseMessageType: {
            return make_pair(
                true,
                static_pointer_cast<Message>(
                    make_shared<RoutingTablesResponse>(messagePart)
                )
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
        mMessagesParser = unique_ptr<MessagesParser>(
            new MessagesParser()
        );

    } catch (std::bad_alloc &) {
        throw MemoryError("IncomingMessagesHandler::IncomingMessagesHandler: "
                              "Сan not allocate enough memory for messages parser.");
    }
}

IncomingMessagesHandler::~IncomingMessagesHandler() {}

void IncomingMessagesHandler::processIncomingMessage(
    udp::endpoint &clientEndpoint,
    const byte *messagePart,
    const size_t receivedBytesCount) {

    if (receivedBytesCount == 0) {
        return;
    }

    if (messagePart == nullptr) {
        throw ValueError("IncomingMessagesHandler::processIncomingMessage: "
                             "Message part can't be null.");
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

        auto channelAndEndpoint = mChannelsManager->incomingChannel(
            *channelNumber,
            clientEndpoint);

        auto bytesBodySharedConst = preparePacketBody(
            mPacketsBuffer.data() + Packet::kPacketBodyOffset,
            *bytesCount - (uint16_t)PacketHeader::kHeaderSize
        );

        Packet::Shared packet;
        try {
            packet = makePacket(
                *channelNumber,
                *packageNumber,
                *totalPacketsCount,
                *bytesCount,
                bytesBodySharedConst
            );

        } catch (bad_alloc &) {
            throw MemoryError("IncomingMessagesHandler::tryCollectPacket: "
                                  "Can not allocate enough memory for incoming packet.");
        }

        // TODO: catch conflict error and log it
        // is needed for network stack analising
        channelAndEndpoint.first->addPacket(
            packet->header()->packetNumber(),
            Packet::Shared(packet)
        );

        cutPacketFromBuffer(*bytesCount);

        tryCollectMessage(
            packet->header()->channelNumber(),
            channelAndEndpoint.first
        );
    }
}

void IncomingMessagesHandler::tryCollectMessage(
    uint16_t channelNumber,
    Channel::Shared channel) {

    if (channel->expectedPacketsCount() == channel->realPacketsCount()) {
        if (channel->checkConsistency()) {
            auto bytesAndCount = channel->data();
            auto resultAndMessage = mMessagesParser->processMessage(
                bytesAndCount.first,
                bytesAndCount.second
            );
            if (resultAndMessage.first) {
                mChannelsManager->removeIncomingChannel(channelNumber);
                messageParsedSignal(resultAndMessage.second);

            } else {
                mChannelsManager->removeIncomingChannel(channelNumber);
            }

        } else {
            mChannelsManager->removeIncomingChannel(channelNumber);
        }
    }
}

ConstBytesShared IncomingMessagesHandler::preparePacketBody(
    byte *bodyPart,
    size_t bodySize) {

    BytesShared bytesBodyShared = tryCalloc(bodySize);
    memcpy(
        bytesBodyShared.get(),
        bodyPart,
        bodySize
    );
    return static_pointer_cast<const byte>(bytesBodyShared);
}

Packet::Shared IncomingMessagesHandler::makePacket(
    uint16_t channelNumber,
    uint16_t packetNumber,
    uint16_t totalPacketsCount,
    uint16_t totalBytesCount,
    ConstBytesShared bytes) {

    return Packet::Shared(
        new Packet(
            makePacketHeader(
                channelNumber,
                packetNumber,
                totalPacketsCount,
                totalBytesCount
            ),
            bytes
        )
    );
}

PacketHeader::Shared IncomingMessagesHandler::makePacketHeader(
    uint16_t channelNumber,
    uint16_t packetNumber,
    uint16_t totalPacketsCount,
    uint16_t totalBytesCount) {

    return PacketHeader::Shared(
        new PacketHeader(
            channelNumber,
            packetNumber,
            totalPacketsCount,
            totalBytesCount)
    );
}

void IncomingMessagesHandler::cutPacketFromBuffer(
    size_t bytesCount) {

    mPacketsBuffer.erase(
        mPacketsBuffer.begin(),
        mPacketsBuffer.begin() + bytesCount
    );
}
