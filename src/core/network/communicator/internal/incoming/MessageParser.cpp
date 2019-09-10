#include "MessageParser.h"

MessagesParser::MessagesParser(
    ContractorsManager *contractorsManager,
    Logger *logger)
    noexcept:

    mContractorsManager(contractorsManager),
    mLog(logger)
{}

pair<bool, Message::Shared> MessagesParser::processBytesSequence(
    BytesShared buffer,
    size_t count)
{
    if (count < kMinimalMessageSize || buffer == nullptr) {
        return messageInvalidOrIncomplete();
    }

    try {
        const SerializedProtocolVersion kMessageProtocolVersion =
            *(reinterpret_cast<SerializedProtocolVersion *>(buffer.get()));
        if (kMessageProtocolVersion != Message::ProtocolVersion::Latest) {
            warning() << "processBytesSequence: Message with invalid protocol version occurred "
                      << (uint16_t)kMessageProtocolVersion << " current protocol version "
                      << Message::Latest << ". Message dropped.";
            return messageInvalidOrIncomplete();
        }

        ContractorID contractorID = *(reinterpret_cast<ContractorID*>(
            buffer.get() + sizeof(SerializedProtocolVersion)));
        try {
            if (contractorID != std::numeric_limits<ContractorID>::max()) {
                auto contractor = mContractorsManager->contractor(contractorID);
#ifdef DEBUG_LOG_NETWORK_COMMUNICATOR
                debug() << "Message encrypted by contractor " << contractorID;
#endif
                auto pair = MsgEncryptor(
                    contractor->cryptoKey()->publicKey,
                    contractor->cryptoKey()->secretKey
                ).decrypt(buffer, count);
                buffer = pair.first;
                if(!buffer) {
                    warning() << "processBytesSequence: "
                        << "Message decryption error. Message dropped.";
                    return messageInvalidOrIncomplete();
                }
            }
        } catch (NotFoundError &) {
            warning() << "There is no contractor with ID " << contractorID;
            return messageInvalidOrIncomplete();
        } catch (std::exception &e) {
            warning() << "Can't decrypt message " << e.what();
            return messageInvalidOrIncomplete();
        }

        const Message::SerializedType kMessageIdentifier =
            *(reinterpret_cast<Message::SerializedType*>(buffer.get() +
                sizeof(ContractorID) + sizeof(SerializedProtocolVersion)));

        switch(kMessageIdentifier) {

        /*
         * System messages
         */
        case Message::System_Confirmation:
            return messageCollected<ConfirmationMessage>(buffer);

        /*
         * Providing messages
         */
        case Message::ProvidingAddressResponse:
            return messageCollected<ProvidingAddressResponseMessage>(buffer);

        /*
         * Channel messages
         */
        case Message::Channel_Init:
            return messageCollected<InitChannelMessage>(buffer);

        case Message::Channel_Confirm:
            return messageCollected<ConfirmChannelMessage>(buffer);

        case Message::Channel_UpdateAddresses:
            return messageCollected<UpdateChannelAddressesMessage>(buffer);

        /*
         * Trust lines messages
         */
        case Message::TrustLines_Initial:
            return messageCollected<TrustLineInitialMessage>(buffer);

        case Message::TrustLines_Confirmation:
            return messageCollected<TrustLineConfirmationMessage>(buffer);

        case Message::TrustLines_PublicKeysSharingInit:
            return messageCollected<PublicKeysSharingInitMessage>(buffer);

        case Message::TrustLines_PublicKey:
            return messageCollected<PublicKeyMessage>(buffer);

        case Message::TrustLines_HashConfirmation:
            return messageCollected<PublicKeyHashConfirmation>(buffer);

        case Message::TrustLines_Audit:
            return messageCollected<AuditMessage>(buffer);

        case Message::TrustLines_AuditConfirmation:
            return messageCollected<AuditResponseMessage>(buffer);

        case Message::TrustLines_Reset:
            return messageCollected<TrustLineResetMessage>(buffer);

        case Message::TrustLines_ConflictResolver:
            return messageCollected<ConflictResolverMessage>(buffer);

        case Message::TrustLines_ConflictResolverConfirmation:
            return messageCollected<ConflictResolverResponseMessage>(buffer);


        /*
         * Payment operations messages
         */
        case Message::Payments_CoordinatorReservationRequest:
            return messageCollected<CoordinatorReservationRequestMessage>(buffer);

        case Message::Payments_CoordinatorReservationResponse:
            return messageCollected<CoordinatorReservationResponseMessage>(buffer);

        case Message::Payments_ReceiverInitPaymentRequest:
            return messageCollected<ReceiverInitPaymentRequestMessage>(buffer);

        case Message::Payments_ReceiverInitPaymentResponse:
            return messageCollected<ReceiverInitPaymentResponseMessage>(buffer);

        case Message::Payments_IntermediateNodeReservationRequest:
            return messageCollected<IntermediateNodeReservationRequestMessage>(buffer);

        case Message::Payments_IntermediateNodeReservationResponse:
            return messageCollected<IntermediateNodeReservationResponseMessage>(buffer);

        case Message::Payments_CoordinatorCycleReservationRequest:
            return messageCollected<CoordinatorCycleReservationRequestMessage>(buffer);

        case Message::Payments_CoordinatorCycleReservationResponse:
            return messageCollected<CoordinatorCycleReservationResponseMessage>(buffer);

        case Message::Payments_IntermediateNodeCycleReservationRequest:
            return messageCollected<IntermediateNodeCycleReservationRequestMessage>(buffer);

        case Message::Payments_IntermediateNodeCycleReservationResponse:
            return messageCollected<IntermediateNodeCycleReservationResponseMessage>(buffer);

        case Message::Payments_ParticipantsVotes:
            return messageCollected<ParticipantsVotesMessage>(buffer);

        case Message::Payments_FinalPathConfiguration:
            return messageCollected<FinalPathConfigurationMessage>(buffer);

        case Message::Payments_FinalPathCycleConfiguration:
            return messageCollected<FinalPathCycleConfigurationMessage>(buffer);

        case Message::Payments_FinalAmountsConfiguration:
            return messageCollected<FinalAmountsConfigurationMessage>(buffer);

        case Message::Payments_FinalAmountsConfigurationResponse:
            return messageCollected<FinalAmountsConfigurationResponseMessage>(buffer);

        case Message::Payments_TTLProlongationRequest:
            return messageCollected<TTLProlongationRequestMessage>(buffer);

        case Message::Payments_TTLProlongationResponse:
            return messageCollected<TTLProlongationResponseMessage>(buffer);

        case Message::Payments_VotesStatusRequest:
            return messageCollected<VotesStatusRequestMessage>(buffer);

        case Message::Payments_TransactionPublicKeyHash:
            return messageCollected<TransactionPublicKeyHashMessage>(buffer);

        case Message::Payments_ParticipantsPublicKeys:
            return messageCollected<ParticipantsPublicKeysMessage>(buffer);

        case Message::Payments_ParticipantVote:
            return messageCollected<ParticipantVoteMessage>(buffer);


        /*
         * Cycles processing messages
         */
        case Message::Cycles_FourNodesNegativeBalanceRequest:
            return messageCollected<CyclesFourNodesNegativeBalanceRequestMessage>(buffer);

        case Message::Cycles_FourNodesPositiveBalanceRequest:
            return messageCollected<CyclesFourNodesPositiveBalanceRequestMessage>(buffer);

        case Message::Cycles_FourNodesBalancesResponse:
            return messageCollected<CyclesFourNodesBalancesResponseMessage>(buffer);

        case Message::Cycles_ThreeNodesBalancesRequest:
            return messageCollected<CyclesThreeNodesBalancesRequestMessage>(buffer);

        case Message::Cycles_ThreeNodesBalancesResponse:
            return messageCollected<CyclesThreeNodesBalancesResponseMessage>(buffer);

        case Message::Cycles_FiveNodesMiddleware:
            return messageCollected<CyclesFiveNodesInBetweenMessage>(buffer);

        case Message::Cycles_FiveNodesBoundary:
            return messageCollected<CyclesFiveNodesBoundaryMessage>(buffer);

        case Message::Cycles_SixNodesMiddleware:
            return messageCollected<CyclesSixNodesInBetweenMessage>(buffer);

        case Message::Cycles_SixNodesBoundary:
            return messageCollected<CyclesSixNodesBoundaryMessage>(buffer);


        /*
         * Max flow calculation messages
         */
        case Message::MaxFlow_InitiateCalculation:
            return messageCollected<InitiateMaxFlowCalculationMessage>(buffer);

        case Message::MaxFlow_ResultMaxFlowCalculation:
            return messageCollected<ResultMaxFlowCalculationMessage>(buffer);

        case Message::MaxFlow_ResultMaxFlowCalculationFromGateway:
            return messageCollected<ResultMaxFlowCalculationGatewayMessage>(buffer);

        case Message::MaxFlow_CalculationSourceFirstLevel:
            return messageCollected<MaxFlowCalculationSourceFstLevelMessage>(buffer);

        case Message::MaxFlow_CalculationTargetFirstLevel:
            return messageCollected<MaxFlowCalculationTargetFstLevelMessage>(buffer);

        case Message::MaxFlow_CalculationSourceSecondLevel:
            return messageCollected<MaxFlowCalculationSourceSndLevelMessage>(buffer);

        case Message::MaxFlow_CalculationTargetSecondLevel:
            return messageCollected<MaxFlowCalculationTargetSndLevelMessage>(buffer);

        case Message::MaxFlow_Confirmation:
            return messageCollected<MaxFlowCalculationConfirmationMessage>(buffer);

        /*
         * Gateway notification & RoutingTables
         */
        case Message::GatewayNotification:
            return messageCollected<GatewayNotificationMessage>(buffer);

        case Message::RoutingTableResponse:
            return messageCollected<RoutingTableResponseMessage>(buffer);

        /*
         * General
         */
        case Message::General_Ping:
            return messageCollected<PingMessage>(buffer);

        case Message::General_Pong:
            return messageCollected<PongMessage>(buffer);

        case Message::General_NoEquivalent:
            return messageCollected<NoEquivalentMessage>(buffer);

        default: {
            warning() << "processBytesSequence: "
                << "Unexpected message identifier occurred (" << kMessageIdentifier << "). Message dropped.";
            return messageInvalidOrIncomplete();
        }

        }

    } catch (exception &) {
        warning() << "processBytesSequence: Unexpected error occurred";
        return messageInvalidOrIncomplete();
    }
}

MessagesParser &MessagesParser::operator=(
    const MessagesParser &other)
    noexcept
{
    mLog = other.mLog;
    return *this;
}

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

string MessagesParser::logHeader()
    noexcept
{
    return "[MessagesParser]";
}

LoggerStream MessagesParser::warning() const
    noexcept
{
    return mLog->warning(logHeader());
}

LoggerStream MessagesParser::debug() const
    noexcept
{
    return mLog->debug(logHeader());
}
