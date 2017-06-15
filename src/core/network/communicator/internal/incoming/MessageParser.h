#ifndef MESSAGEPARSER_H
#define MESSAGEPARSER_H

#include "../../../../common/Types.h"
#include "../../../../common/memory/MemoryUtils.h"

// ToDo: remove this message type
#include "../../../messages/response/Response.h"

// ToDo: reorganize this messages
#include "../../../messages/trust_lines/AcceptTrustLineMessage.h"
#include "../../../messages/trust_lines/UpdateTrustLineMessage.h"
#include "../../../messages/trust_lines/RejectTrustLineMessage.h"

#include "../../../messages/routing_tables/NotificationTrustLineCreatedMessage.h"
#include "../../../messages/routing_tables/NotificationTrustLineRemovedMessage.h"
#include "../../../messages/routing_tables/NeighborsRequestMessage.h"
#include "../../../messages/routing_tables/NeighborsResponseMessage.h"
#include "../../../messages/routing_tables/CRC32Rt2ResponseMessage.h"
#include "../../../messages/routing_tables/CRC32Rt2RequestMessage.h"



#include "../../../messages/max_flow_calculation/InitiateMaxFlowCalculationMessage.h"
#include "../../../messages/max_flow_calculation/MaxFlowCalculationSourceFstLevelMessage.h"
#include "../../../messages/max_flow_calculation/MaxFlowCalculationTargetFstLevelMessage.h"
#include "../../../messages/max_flow_calculation/MaxFlowCalculationSourceSndLevelMessage.h"
#include "../../../messages/max_flow_calculation/MaxFlowCalculationTargetSndLevelMessage.h"
#include "../../../messages/max_flow_calculation/ResultMaxFlowCalculationMessage.h"

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
#include "../../../messages/payments/TTLPolongationMessage.h"

#include "../../../messages/total_balances/InitiateTotalBalancesMessage.h"
#include "../../../messages/total_balances/TotalBalancesResultMessage.h"

#include "../../../messages/find_path/RequestRoutingTablesMessage.h"
#include "../../../messages/find_path/ResultRoutingTable1LevelMessage.h"
#include "../../../messages/find_path/ResultRoutingTable2LevelMessage.h"
#include "../../../messages/find_path/ResultRoutingTable3LevelMessage.h"

#include "../../../messages/cycles/SixAndFiveNodes/CyclesFiveNodesInBetweenMessage.hpp"
#include "../../../messages/cycles/SixAndFiveNodes/CyclesSixNodesInBetweenMessage.hpp"
#include "../../../messages/cycles/SixAndFiveNodes/CyclesFiveNodesBoundaryMessage.hpp"
#include "../../../messages/cycles/SixAndFiveNodes/CyclesSixNodesBoundaryMessage.hpp"
#include "../../../messages/cycles/ThreeNodes/CyclesThreeNodesBalancesRequestMessage.h"
#include "../../../messages/cycles/ThreeNodes/CyclesThreeNodesBalancesResponseMessage.h"
#include "../../../messages/cycles/FourNodes/CyclesFourNodesBalancesRequestMessage.h"
#include "../../../messages/cycles/FourNodes/CyclesFourNodesBalancesResponseMessage.h"

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
    Logger *mLog;
};



#endif // MESSAGEPARSER_H
