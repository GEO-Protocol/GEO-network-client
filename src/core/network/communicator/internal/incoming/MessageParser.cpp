#include "MessageParser.h"


MessagesParser::MessagesParser(
    Logger &logger)
    noexcept:

    mLog(logger)
{}

pair<bool, Message::Shared> MessagesParser::processBytesSequence(
    BytesShared bytes,
    const size_t count) {

    if (count < kMinimalMessageSize || bytes == nullptr)
        return messageInvalidOrIncomplete();

    try {
        const Message::SerializedType kMessageIdentifier = *bytes;

        switch(kMessageIdentifier) {

        /*
         * Trust lines operations
         */
        case Message::TrustLines_Open:
            return messageCollected<AcceptTrustLineMessage>(bytes);

        case Message::TrustLines_Close:
            return messageCollected<RejectTrustLineMessage>(bytes);

        case Message::TrustLines_Set:
            return messageCollected<UpdateTrustLineMessage>(bytes);


        /*
         * ToDo: Add routing tables messages
         */


        /*
         * Payment operations messages
         */
        case Message::Payments_CoordinatorReservationRequest:
            return messageCollected<CoordinatorReservationRequestMessage>(bytes);

        case Message::Payments_CoordinatorReservationResponse:
            return messageCollected<CoordinatorReservationResponseMessage>(bytes);

        case Message::Payments_ReceiverInitPaymentRequest:
            return messageCollected<ReceiverInitPaymentRequestMessage>(bytes);

        case Message::Payments_ReceiverInitPaymentResponse:
            return messageCollected<ReceiverInitPaymentResponseMessage>(bytes);

        case Message::Payments_IntermediateNodeReservationRequest:
            return messageCollected<IntermediateNodeReservationRequestMessage>(bytes);

        case Message::Payments_IntermediateNodeReservationResponse:
            return messageCollected<IntermediateNodeReservationResponseMessage>(bytes);


            /*
             * Cycles processing messages
             */
        case Message::Cycles_SixNodesMiddleware:
            return messageCollected<CyclesSixNodesInBetweenMessage>(bytes);

        case Message::Cycles_FiveNodesMiddleware:
            return messageCollected<CyclesFiveNodesInBetweenMessage>(bytes);

        case Message::Cycles_SixNodesBoundary:
            return messageCollected<CyclesSixNodesBoundaryMessage>(bytes);

        case Message::Cycles_FiveNodesBoundary:
            return messageCollected<CyclesFiveNodesBoundaryMessage>(bytes);

        case Message::Cycles_ThreeNodesBalancesResponse:
            return messageCollected<CyclesThreeNodesBalancesResponseMessage>(bytes);

        case Message::Cycles_FourNodesBalancesRequest:
            return messageCollected<CyclesFourNodesBalancesRequestMessage>(bytes);

        case Message::Cycles_FourNodesBalancesResponse:
            return messageCollected<CyclesFourNodesBalancesResponseMessage>(bytes);

        case Message::Cycles_ThreeNodesBalancesRequest:
            return messageCollected<CyclesThreeNodesBalancesRequestMessage>(bytes);


        /*
         * Max flow calculation messages
         */
        case Message::MaxFlow_InitiateCalculation:
            return messageCollected<InitiateMaxFlowCalculationMessage>(bytes);

        case Message::MaxFlow_ResultMaxFlowCalculation:
            return messageCollected<ResultMaxFlowCalculationMessage>(bytes);

        case Message::MaxFlow_CalculationSourceFirstLevel:
            return messageCollected<MaxFlowCalculationSourceFstLevelMessage>(bytes);

        case Message::MaxFlow_CalculationTargetFirstLevel:
            return messageCollected<MaxFlowCalculationTargetFstLevelMessage>(bytes);

        case Message::MaxFlow_CalculationSourceSecondLevel:
            return messageCollected<MaxFlowCalculationSourceSndLevelMessage>(bytes);

        case Message::MaxFlow_CalculationTargetSecondLevel:
            return messageCollected<MaxFlowCalculationTargetSndLevelMessage>(bytes);


        /*
         * Total Balances Messages
         */
        case Message::TotalBalance_Request:
            return messageCollected<InitiateTotalBalancesMessage>(bytes);

        case Message::TotalBalance_Response:
            return messageCollected<TotalBalancesResultMessage>(bytes);


        /*
         * Find Path Messages
         */
        case Message::Paths_RequestRoutingTables:
            return messageCollected<RequestRoutingTablesMessage>(bytes);

        case Message::Paths_ResultRoutingTableFirstLevel:
            return messageCollected<ResultRoutingTable1LevelMessage>(bytes);

        case Message::Paths_ResultRoutingTableSecondLevel:
            return messageCollected<ResultRoutingTable2LevelMessage>(bytes);

        case Message::Paths_ResultRoutingTableThirdLevel:
            return messageCollected<ResultRoutingTable3LevelMessage>(bytes);


        default: {
            auto errors = mLog.error("MessagesParser::processBytesSequence");
            errors << "Unexpected message identifier occured (" << kMessageIdentifier << "). Message dropped.";
        }
        }

    } catch (exception &e) {
        return messageInvalidOrIncomplete();
        auto errors = mLog.error("MessagesParser::processBytesSequence");
        errors << "Message can't be parsed: " << e.what();
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
