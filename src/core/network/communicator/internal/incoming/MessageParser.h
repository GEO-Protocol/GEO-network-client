#ifndef MESSAGEPARSER_H
#define MESSAGEPARSER_H

#include "../../../../common/memory/MemoryUtils.h"
#include "../../../../crypto/MsgEncryptor.h"

#include "../../../messages/base/transaction/ConfirmationMessage.h"

#include "../../../messages/trust_line_channels/InitChannelMessage.h"

#include "../../../messages/trust_lines/TrustLineInitialMessage.h"
#include "../../../messages/trust_lines/TrustLineConfirmationMessage.h"
#include "../../../messages/trust_lines/PublicKeysSharingInitMessage.h"
#include "../../../messages/trust_lines/PublicKeyMessage.h"
#include "../../../messages/trust_lines/PublicKeyHashConfirmation.h"
#include "../../../messages/trust_lines/AuditMessage.h"
#include "../../../messages/trust_lines/AuditResponseMessage.h"
#include "../../../messages/trust_lines/ConflictResolverMessage.h"
#include "../../../messages/trust_lines/ConflictResolverResponseMessage.h"

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
#include "../../../messages/payments/ParticipantsPublicKeysMessage.h"
#include "../../../messages/payments/ParticipantVoteMessage.h"
#include "../../../messages/payments/TransactionPublicKeyHashMessage.h"

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
#include "../../../messages/gateway_notification_and_routing_tables/RoutingTableResponseMessage.h"

#include "../../../messages/general/PingMessage.h"
#include "../../../messages/general/PongMessage.h"
#include "../../../messages/general/NoEquivalentMessage.h"

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
        size_t count);

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

    LoggerStream debug() const
    noexcept;

protected:
    Logger *mLog;
};



#endif // MESSAGEPARSER_H
