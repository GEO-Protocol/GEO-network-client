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
//    cout << "_________________________" << endl;
//    cout << "MessagesParser::tryDeserializeMessage " << endl;
    try {
        uint16_t *messageIdentifier = new (messagePart.get()) uint16_t;
//        cout << messageIdentifier << ";" << *messageIdentifier << endl;
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

        case Message::TrustLines_Open: {
            return make_pair(
                true,
                static_pointer_cast<Message>(
                    make_shared<AcceptTrustLineMessage>(messagePart)
                )
            );
        }

        case Message::TrustLines_Close: {
            return make_pair(
                true,
                static_pointer_cast<Message>(
                    make_shared<RejectTrustLineMessage>(messagePart)
                )
            );
        }

        case Message::TrustLines_Set: {
            return make_pair(
                true,
                static_pointer_cast<Message>(
                    make_shared<UpdateTrustLineMessage>(messagePart)
                )
            );
        }

        /*
         * Routing tables exchange messages
         */
        case Message::RoutingTables_NotificationTrustLineCreated:
            return messageCollected<NotificationTrustLineCreatedMessage>(messagePart);

        case Message::RoutingTables_NotificationTrustLineRemoved:
            return messageCollected<NotificationTrustLineRemovedMessage>(messagePart);

        case Message::RoutingTables_NeighborsRequest:
            return messageCollected<NeighborsRequestMessage>(messagePart);

        case Message::RoutingTables_NeighborsResponse:
            return messageCollected<NeighborsResponseMessage>(messagePart);

        /*
         * Payment operations messages
         */
        case Message::Payments_CoordinatorReservationRequest:
            return messageCollected<CoordinatorReservationRequestMessage>(messagePart);

        case Message::Payments_CoordinatorReservationResponse:
            return messageCollected<CoordinatorReservationResponseMessage>(messagePart);

        case Message::Payments_ReceiverInitPaymentRequest:
            return messageCollected<ReceiverInitPaymentRequestMessage>(messagePart);

        case Message::Payments_ReceiverInitPaymentResponse:
            return messageCollected<ReceiverInitPaymentResponseMessage>(messagePart);

        case Message::Payments_IntermediateNodeReservationRequest:
            return messageCollected<IntermediateNodeReservationRequestMessage>(messagePart);

        case Message::Payments_IntermediateNodeReservationResponse:
            return messageCollected<IntermediateNodeReservationResponseMessage>(messagePart);

        case Message::Payments_ParticipantsVotes:
            return messageCollected<ParticipantsVotesMessage>(messagePart);

        case Message::Payments_ParticipantsPathsConfigurationRequest:
            return messageCollected<ParticipantsConfigurationRequestMessage>(messagePart);

        case Message::Payments_ParticipantsPathsConfiguration:
            return messageCollected<ParticipantsConfigurationMessage>(messagePart);

        case Message::Payments_FinalPathConfiguration:
            return messageCollected<FinalPathConfigurationMessage>(messagePart);

        /*
         * Cycles processing messages
         */
        case Message::Cycles_SixNodesMiddleware: {
            return make_pair(
                true,
                static_pointer_cast<Message>(
                    make_shared<CyclesSixNodesInBetweenMessage>(messagePart)
                )
            );
        }
        case Message::Cycles_FiveNodesMiddleware: {
            return make_pair(
                true,
                static_pointer_cast<Message>(
                    make_shared<CyclesFiveNodesInBetweenMessage>(messagePart)
                )
            );
        }
        case Message::Cycles_SixNodesBoundary: {
            return make_pair(
                true,
                static_pointer_cast<Message>(
                    make_shared<CyclesSixNodesBoundaryMessage>(messagePart)
                )
            );
        }
        case Message::Cycles_FiveNodesBoundary: {
            return make_pair(
                true,
                static_pointer_cast<Message>(
                    make_shared<CyclesFiveNodesBoundaryMessage>(messagePart)
                )
            );
        }
        case Message::Cycles_ThreeNodesBalancesResponse: {
            return make_pair(
                true,
                static_pointer_cast<Message>(
                    make_shared<CyclesThreeNodesBalancesResponseMessage>(messagePart)
                )
            );
        }
        case Message::Cycles_FourNodesBalancesRequest: {
            return make_pair(
                    true,
                    static_pointer_cast<Message>(
                            make_shared<CyclesFourNodesBalancesRequestMessage>(messagePart)
                    )
            );
        }
        case Message::Cycles_FourNodesBalancesResponse: {
            return make_pair(
                    true,
                    static_pointer_cast<Message>(
                            make_shared<CyclesFourNodesBalancesResponseMessage>(messagePart)
                    )
            );
        }
        case Message::Cycles_ThreeNodesBalancesRequest: {
            return make_pair(
                true,
                static_pointer_cast<Message>(
                    make_shared<CyclesThreeNodesBalancesRequestMessage>(messagePart)
                )
            );
        }
        /*
         * Max flow calculation messages
         */
        case Message::MaxFlow_InitiateCalculation: {
            return make_pair(
                true,
                static_pointer_cast<Message>(
                    make_shared<InitiateMaxFlowCalculationMessage>(messagePart)));
        }

        case Message::MaxFlow_ResultMaxFlowCalculation: {
            return make_pair(
                true,
                static_pointer_cast<Message>(
                    make_shared<ResultMaxFlowCalculationMessage>(messagePart)));
        }

        case Message::MaxFlow_CalculationSourceFirstLevel: {
            return make_pair(
                true,
                static_pointer_cast<Message>(
                    make_shared<MaxFlowCalculationSourceFstLevelMessage>(messagePart)));
        }

        case Message::MaxFlow_CalculationTargetFirstLevel: {
            return make_pair(
                true,
                static_pointer_cast<Message>(
                    make_shared<MaxFlowCalculationTargetFstLevelMessage>(messagePart)));
        }

        case Message::MaxFlow_CalculationSourceSecondLevel: {
            return make_pair(
                true,
                static_pointer_cast<Message>(
                    make_shared<MaxFlowCalculationSourceSndLevelMessage>(messagePart)));
        }

        case Message::MaxFlow_CalculationTargetSecondLevel: {
            return make_pair(
                true,
                static_pointer_cast<Message>(
                    make_shared<MaxFlowCalculationTargetSndLevelMessage>(messagePart)));
        }

        /*
         * Total Balances Messages
         */
        case Message::TotalBalance_Request: {
            return make_pair(
                true,
                static_pointer_cast<Message>(
                    make_shared<InitiateTotalBalancesMessage>(messagePart)));
        }

        case Message::TotalBalance_Response: {
            return make_pair(
                true,
                static_pointer_cast<Message>(
                    make_shared<TotalBalancesResultMessage>(messagePart)));
        }

        /*
         * Find Path Messages
         */
        case Message::Paths_RequestRoutingTables: {
            return make_pair(
                true,
                static_pointer_cast<Message>(
                    make_shared<RequestRoutingTablesMessage>(messagePart)));
        }

        case Message::Paths_ResultRoutingTableFirstLevel: {
            return make_pair(
                true,
                static_pointer_cast<Message>(
                    make_shared<ResultRoutingTable1LevelMessage>(messagePart)));
        }

        case Message::Paths_ResultRoutingTableSecondLevel: {
            return make_pair(
                true,
                static_pointer_cast<Message>(
                    make_shared<ResultRoutingTable2LevelMessage>(messagePart)));
        }

        case Message::Paths_ResultRoutingTableThirdLevel: {
            return make_pair(
                true,
                static_pointer_cast<Message>(
                    make_shared<ResultRoutingTable3LevelMessage>(messagePart)));
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

        case Message::MessageType::ResponseMessageType: {
            return make_pair(
                true,
                static_pointer_cast<Message>(
                    make_shared<Response>(messagePart)
                )
            );
        }

        default: {
            return messageInvalidOrIncomplete();
        }
    }
};

pair<bool, Message::Shared> MessagesParser::messageInvalidOrIncomplete()
{
    return make_pair(
        false,
        Message::Shared(nullptr));
}

template <class CollectedMessageType>
pair<bool, Message::Shared> MessagesParser::messageCollected(
    CollectedMessageType message) const
{
    return make_pair(
        true,
        static_pointer_cast<Message>(
            make_shared<CollectedMessageType>(message)));
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
            if (*channelNumber != previousChannel) {
                previousChannel = *channelNumber;
                realPacketNumber = 0;
            }

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
            channelAndEndpoint.second,
            packet->header()->channelNumber(),
            channelAndEndpoint.first
        );
    }
}

void IncomingMessagesHandler::tryCollectMessage(
    udp::endpoint endpoint,
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
                mChannelsManager->removeIncomingChannel(
                    endpoint,
                    channelNumber);
                messageParsedSignal(resultAndMessage.second);

            } else {
                mChannelsManager->removeIncomingChannel(
                    endpoint,
                    channelNumber);
            }

        } else {
            mChannelsManager->removeIncomingChannel(
                endpoint,
                channelNumber);
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
