#include "MessageParser.h"


MessagesParser::MessagesParser(
    Logger *logger)
    noexcept:

    mLog(logger)
{}

pair<bool, Message::Shared> MessagesParser::processBytesSequence(
    BytesShared buffer,
    const size_t count) {

    if (count < kMinimalMessageSize || buffer == nullptr) {
        return messageInvalidOrIncomplete();
    }

    try {
        const Message::SerializedType kMessageIdentifier =
            *(reinterpret_cast<Message::SerializedType*>(buffer.get()));

        switch(kMessageIdentifier) {

        /*
         * Trust lines operations
         */
        case Message::TrustLines_Open:
            // ToDo: Replace it with proper request message
            // (OpenTrustLineRequestMessage)
            return messageCollected<AcceptTrustLineMessage>(buffer);

        case Message::TrustLines_Close:
            // ToDo: Replace it with proper request message
            // (CloseTrustLineRequestMessage)
            return messageCollected<RejectTrustLineMessage>(buffer);

        case Message::TrustLines_Set:
            // ToDo: Replace it with proper request message
            // (SetTrustLineRequestMessage)
            return messageCollected<UpdateTrustLineMessage>(buffer);


        /*
         * ToDo: Add routing tables messages
         */
        case Message::RoutingTables_NotificationTrustLineCreated:
            return messageCollected<NotificationTrustLineCreatedMessage>(buffer);

        case Message::RoutingTables_NotificationTrustLineRemoved:
            return messageCollected<NotificationTrustLineRemovedMessage>(buffer);

        case Message::RoutingTables_NeighborsRequest:
            return messageCollected<NeighborsRequestMessage>(buffer);

        case Message::RoutingTables_NeighborsResponse:
            return messageCollected<NeighborsResponseMessage>(buffer);

        case Message::RoutingTables_CRC32Rt2ResponseMessage:
            return messageCollected<CRC32Rt2ResponseMessage>(buffer);

        case Message::RoutingTables_CRC32Rt2RequestMessage:
            return messageCollected<CRC32Rt2RequestMessage>(buffer);


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

        case Message::Payments_TTLProlongation:
            return messageCollected<TTLPolongationMessage>(buffer);


        /*
         * Cycles processing messages
         */
        case Message::Cycles_SixNodesMiddleware:
            return messageCollected<CyclesSixNodesInBetweenMessage>(buffer);

        case Message::Cycles_FiveNodesMiddleware:
            return messageCollected<CyclesFiveNodesInBetweenMessage>(buffer);

        case Message::Cycles_SixNodesBoundary:
            return messageCollected<CyclesSixNodesBoundaryMessage>(buffer);

        case Message::Cycles_FiveNodesBoundary:
            return messageCollected<CyclesFiveNodesBoundaryMessage>(buffer);

        case Message::Cycles_ThreeNodesBalancesResponse:
            return messageCollected<CyclesThreeNodesBalancesResponseMessage>(buffer);

        case Message::Cycles_FourNodesBalancesRequest:
            return messageCollected<CyclesFourNodesBalancesRequestMessage>(buffer);

        case Message::Cycles_FourNodesBalancesResponse:
            return messageCollected<CyclesFourNodesBalancesResponseMessage>(buffer);

        case Message::Cycles_ThreeNodesBalancesRequest:
            return messageCollected<CyclesThreeNodesBalancesRequestMessage>(buffer);


        /*
         * Max flow calculation messages
         */
        case Message::MaxFlow_InitiateCalculation:
            return messageCollected<InitiateMaxFlowCalculationMessage>(buffer);

        case Message::MaxFlow_ResultMaxFlowCalculation:
            return messageCollected<ResultMaxFlowCalculationMessage>(buffer);

        case Message::MaxFlow_CalculationSourceFirstLevel:
            return messageCollected<MaxFlowCalculationSourceFstLevelMessage>(buffer);

        case Message::MaxFlow_CalculationTargetFirstLevel:
            return messageCollected<MaxFlowCalculationTargetFstLevelMessage>(buffer);

        case Message::MaxFlow_CalculationSourceSecondLevel:
            return messageCollected<MaxFlowCalculationSourceSndLevelMessage>(buffer);

        case Message::MaxFlow_CalculationTargetSecondLevel:
            return messageCollected<MaxFlowCalculationTargetSndLevelMessage>(buffer);


        /*
         * Total Balances Messages
         */
        case Message::TotalBalance_Request:
            return messageCollected<InitiateTotalBalancesMessage>(buffer);

        case Message::TotalBalance_Response:
            return messageCollected<TotalBalancesResultMessage>(buffer);


        /*
         * Find Path Messages
         */
        case Message::Paths_RequestRoutingTables:
            return messageCollected<RequestRoutingTablesMessage>(buffer);

        case Message::Paths_ResultRoutingTableFirstLevel:
            return messageCollected<ResultRoutingTable1LevelMessage>(buffer);

        case Message::Paths_ResultRoutingTableSecondLevel:
            return messageCollected<ResultRoutingTable2LevelMessage>(buffer);

        case Message::Paths_ResultRoutingTableThirdLevel:
            return messageCollected<ResultRoutingTable3LevelMessage>(buffer);


        /*
         * Response message
         * ToDo: remove it after trust lines messages refactoring
         */
        case Message::ResponseMessageType:
            return messageCollected<Response>(buffer);


#ifdef DEBUG
        /*
         * Debug messages
         */
        case Message::Debug:
            return messageCollected<DebugMessage>(buffer);
#endif

        default: {
            mLog->error("MessagesParser::processBytesSequence")
                << "Unexpected message identifier occured (" << kMessageIdentifier << "). Message dropped.";
            return messageInvalidOrIncomplete();
        }

        }

    } catch (exception &) {
        return messageInvalidOrIncomplete();
    }
}

MessagesParser &MessagesParser::operator=(
    const MessagesParser &other)
    noexcept
{
    mLog = other.mLog;
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
