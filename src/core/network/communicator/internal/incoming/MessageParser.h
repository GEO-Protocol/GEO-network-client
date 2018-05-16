#ifndef MESSAGEPARSER_H
#define MESSAGEPARSER_H

#include "../../../../common/memory/MemoryUtils.h"

#include "../../../messages/base/transaction/ConfirmationMessage.h"

#include "../../../messages/trust_lines/SetIncomingTrustLineMessage.h"
#include "../../../messages/trust_lines/SetIncomingTrustLineFromGatewayMessage.h"
#include "../../../messages/trust_lines/CloseOutgoingTrustLineMessage.h"

#include "../../../messages/max_flow_calculation/InitiateMaxFlowCalculationMessage.h"
#include "../../../messages/max_flow_calculation/MaxFlowCalculationSourceFstLevelMessage.h"
#include "../../../messages/max_flow_calculation/MaxFlowCalculationTargetFstLevelMessage.h"
#include "../../../messages/max_flow_calculation/MaxFlowCalculationSourceSndLevelMessage.h"
#include "../../../messages/max_flow_calculation/MaxFlowCalculationTargetSndLevelMessage.h"
#include "../../../messages/max_flow_calculation/ResultMaxFlowCalculationMessage.h"
#include "../../../messages/max_flow_calculation/ResultMaxFlowCalculationGatewayMessage.h"

#include "../../../messages/payments/CoordinatorReservationRequestMessage.h"
#include "../../../messages/payments/CoordinatorReservationResponseMessage.h"
#include "../../../messages/payments/IntermediateNodeReservationRequestMessage.h"
#include "../../../messages/payments/IntermediateNodeReservationResponseMessage.h"
#include "../../../messages/payments/ReceiverInitPaymentRequestMessage.h"
#include "../../../messages/payments/ReceiverInitPaymentResponseMessage.h"
#include "../../../messages/payments/CoordinatorCycleReservationRequestMessage.h"
#include "../../../messages/payments/CoordinatorCycleReservationResponseMessage.h"
#include "../../../messages/payments/IntermediateNodeCycleReservationRequestMessage.h"
#include "../../../messages/payments/IntermediateNodeCycleReservationResponseMessage.h"
#include "../../../messages/payments/ParticipantsVotesMessage.h"
#include "../../../messages/payments/FinalPathConfigurationMessage.h"
#include "../../../messages/payments/FinalPathCycleConfigurationMessage.h"
#include "../../../messages/payments/TTLProlongationRequestMessage.h"
#include "../../../messages/payments/TTLProlongationResponseMessage.h"
#include "../../../messages/payments/VotesStatusRequestMessage.hpp"
#include "../../../messages/payments/FinalAmountsConfigurationMessage.h"
#include "../../../messages/payments/FinalAmountsConfigurationResponseMessage.h"
#include "../../../messages/payments/ReservationsInRelationToNodeMessage.h"

#include "../../../messages/cycles/ThreeNodes/CyclesThreeNodesBalancesRequestMessage.h"
#include "../../../messages/cycles/ThreeNodes/CyclesThreeNodesBalancesResponseMessage.h"
#include "../../../messages/cycles/FourNodes/CyclesFourNodesNegativeBalanceRequestMessage.h"
#include "../../../messages/cycles/FourNodes/CyclesFourNodesPositiveBalanceRequestMessage.h"
#include "../../../messages/cycles/FourNodes/CyclesFourNodesBalancesResponseMessage.h"
#include "../../../messages/cycles/SixAndFiveNodes/CyclesFiveNodesInBetweenMessage.hpp"
#include "../../../messages/cycles/SixAndFiveNodes/CyclesSixNodesInBetweenMessage.hpp"
#include "../../../messages/cycles/SixAndFiveNodes/CyclesFiveNodesBoundaryMessage.hpp"
#include "../../../messages/cycles/SixAndFiveNodes/CyclesSixNodesBoundaryMessage.hpp"

#include "../../../messages/gateway_notification_and_routing_tables/GatewayNotificationMessage.h"
#include "../../../messages/gateway_notification_and_routing_tables/GatewayNotificationOneEquivalentMessage.h"
#include "../../../messages/gateway_notification_and_routing_tables/RoutingTableResponseMessage.h"

#ifdef DEBUG
#include "../../../messages/debug/DebugMessage.h"
#endif

#include "../../../../logger/Logger.h"

#include <utility>


using namespace std;


class MessagesParser {
public:
    MessagesParser(
        Logger *logger)
        noexcept;

    pair<bool, Message::Shared> processBytesSequence(
        BytesShared buffer,
        const size_t count);

    MessagesParser& operator= (
        const MessagesParser &other)
        noexcept;

protected:
    const size_t kMessageIdentifierSize = 2;
    const size_t kMinimalMessageSize = kMessageIdentifierSize + 1;

protected:
    pair<bool, Message::Shared> messageInvalidOrIncomplete();

    template <class CollectedMessageType>
    pair<bool, Message::Shared> messageCollected(
        CollectedMessageType message) const;

protected:
    static string logHeader()
    noexcept;

    LoggerStream warning() const
    noexcept;

protected:
    Logger *mLog;
};



#endif // MESSAGEPARSER_H
