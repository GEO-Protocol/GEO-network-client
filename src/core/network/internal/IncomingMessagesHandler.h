#ifndef GEO_NETWORK_CLIENT_INCOMINGCONNECTIONSHANDLER_H
#define GEO_NETWORK_CLIENT_INCOMINGCONNECTIONSHANDLER_H


#include "../channels/packet/Packet.h"
#include "../channels/channel/Channel.h"
#include "../channels/manager/ChannelsManager.h"

#include "../messages/trust_lines/AcceptTrustLineMessage.h"
#include "../messages/trust_lines/UpdateTrustLineMessage.h"
#include "../messages/trust_lines/RejectTrustLineMessage.h"

// todo: include routing table

#include "../messages/max_flow_calculation/InitiateMaxFlowCalculationMessage.h"
#include "../messages/max_flow_calculation/MaxFlowCalculationSourceFstLevelMessage.h"
#include "../messages/max_flow_calculation/MaxFlowCalculationTargetFstLevelMessage.h"
#include "../messages/max_flow_calculation/MaxFlowCalculationSourceSndLevelMessage.h"
#include "../messages/max_flow_calculation/MaxFlowCalculationTargetSndLevelMessage.h"
#include "../messages/max_flow_calculation/ResultMaxFlowCalculationMessage.h"
#include "../messages/payments/CoordinatorReservationRequestMessage.h"
#include "../messages/payments/CoordinatorReservationResponseMessage.h"
#include "../messages/payments/IntermediateNodeReservationRequestMessage.h"
#include "../messages/payments/IntermediateNodeReservationResponseMessage.h"
#include "../messages/payments/ReceiverInitPaymentRequestMessage.h"
#include "../messages/payments/ReceiverInitPaymentResponseMessage.h"
#include "../messages/payments/ParticipantsVotesMessage.h"
#include "../messages/payments/ParticipantsConfigurationRequestMessage.h"
#include "../messages/payments/ParticipantsConfigurationMessage.h"
#include "../messages/response/Response.h"
#include "../messages/total_balances/InitiateTotalBalancesMessage.h"
#include "../messages/total_balances/TotalBalancesResultMessage.h"
#include "../messages/find_path/RequestRoutingTablesMessage.h"
#include "../messages/find_path/ResultRoutingTable1LevelMessage.h"
#include "../messages/find_path/ResultRoutingTable2LevelMessage.h"
#include "../messages/find_path/ResultRoutingTable3LevelMessage.h"

#include "../messages/cycles/SixAndFiveNodes/CyclesFiveNodesInBetweenMessage.hpp"
#include "../messages/cycles/SixAndFiveNodes/CyclesSixNodesInBetweenMessage.hpp"
#include "../messages/cycles/SixAndFiveNodes/CyclesFiveNodesBoundaryMessage.hpp"
#include "../messages/cycles/SixAndFiveNodes/CyclesSixNodesBoundaryMessage.hpp"
#include "../messages/cycles/ThreeNodes/CyclesThreeNodesBalancesRequestMessage.h"
#include "../messages/cycles/ThreeNodes/CyclesThreeNodesBalancesResponseMessage.h"
#include "../messages/cycles/FourNodes/CyclesFourNodesBalancesRequestMessage.h"
#include "../messages/cycles/FourNodes/CyclesFourNodesBalancesResponseMessage.h"

#include "../../common/exceptions/ValueError.h"
#include "../../common/exceptions/ConflictError.h"

#include <boost/asio.hpp>
#include <boost/signals2.hpp>


using namespace std;
using namespace boost::asio::ip;

namespace signals = boost::signals2;

class MessagesParser {

public:
    pair<bool, Message::Shared> processMessage(
        BytesShared messagePart,
        const size_t receivedBytesCount);

private:
    pair<bool, Message::Shared> tryDeserializeMessage(
        BytesShared messagePart);

    // todo: (dm) rename to tryDeserializeMessage.
    pair<bool, Message::Shared> tryDeserializeRequest(
        const uint16_t messageIdentifier,
        BytesShared messagePart);

    pair<bool, Message::Shared> tryDeserializeResponse(
        const uint16_t messageIdentifier,
        BytesShared messagePart);

    pair<bool, Message::Shared> messageInvalidOrIncomplete();

    template <class CollectedMessageType>
    pair<bool, Message::Shared> messageCollected(
        CollectedMessageType message) const;

private:
    const size_t kMessageIdentifierSize = 2;
    const size_t kMinimalMessageSize = kMessageIdentifierSize + 1;

};


class IncomingMessagesHandler {
public:
    signals::signal<void(Message::Shared)> messageParsedSignal;

public:
    IncomingMessagesHandler(
        ChannelsManager *channelsManager);

    ~IncomingMessagesHandler();

    void processIncomingMessage(
        udp::endpoint &clientEndpoint,
        const byte *messagePart,
        const size_t receivedBytesCount);

private:
    void tryCollectPacket(
        udp::endpoint &clientEndpoint);

    void tryCollectMessage(
        udp::endpoint endpoint,
        uint16_t channelNumber,
        Channel::Shared channel);

    ConstBytesShared preparePacketBody(
        byte *bodyPart,
        size_t bodySize);

    Packet::Shared makePacket(
        uint16_t channelNumber,
        uint16_t packetNumber,
        uint16_t totalPacketsCount,
        uint16_t totalBytesCount,
        ConstBytesShared bytes
    );

    PacketHeader::Shared makePacketHeader(
        uint16_t channelNumber,
        uint16_t packetNumber,
        uint16_t totalPacketsCount,
        uint16_t totalBytesCount);

    void cutPacketFromBuffer(
        size_t bytesCount);

private:
    ChannelsManager *mChannelsManager;

    unique_ptr<MessagesParser> mMessagesParser;
    vector<byte> mPacketsBuffer;
};

#endif //GEO_NETWORK_CLIENT_INCOMINGCONNECTIONSHANDLER_H
