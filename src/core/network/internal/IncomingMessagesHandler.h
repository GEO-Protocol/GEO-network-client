#ifndef GEO_NETWORK_CLIENT_INCOMINGCONNECTIONSHANDLER_H
#define GEO_NETWORK_CLIENT_INCOMINGCONNECTIONSHANDLER_H

#include "../../common/Types.h"
#include "../../common/memory/MemoryUtils.h"

#include "../channels/packet/PacketHeader.h"
#include "../channels/packet/Packet.h"
#include "../channels/channel/Channel.h"
#include "../channels/manager/ChannelsManager.h"

#include "../messages/Message.hpp"
#include "../messages/incoming/trust_lines/AcceptTrustLineMessage.h"
#include "../messages/incoming/trust_lines/UpdateTrustLineMessage.h"
#include "../messages/incoming/trust_lines/RejectTrustLineMessage.h"
#include "../messages/incoming/routing_tables/FirstLevelRoutingTableIncomingMessage.h"
#include "../messages/incoming/routing_tables/SecondLevelRoutingTableIncomingMessage.h"
#include "../messages/incoming/max_flow_calculation/ReceiveMaxFlowCalculationOnTargetMessage.h"
#include "../messages/incoming/max_flow_calculation/ResultMaxFlowCalculationFromTargetMessage.h"
#include "../messages/incoming/max_flow_calculation/MaxFlowCalculationSourceFstLevelInMessage.h"
#include "../messages/incoming/max_flow_calculation/MaxFlowCalculationTargetFstLevelInMessage.h"
#include "../messages/incoming/max_flow_calculation/MaxFlowCalculationSourceSndLevelInMessage.h"
#include "../messages/incoming/max_flow_calculation/MaxFlowCalculationTargetSndLevelInMessage.h"
#include "../messages/incoming/max_flow_calculation/ResultMaxFlowCalculationFromSourceMessage.h"
#include "../messages/outgoing/payments/CoordinatorReservationRequestMessage.h"
#include "../messages/outgoing/payments/CoordinatorReservationResponseMessage.h"
#include "../messages/outgoing/payments/IntermediateNodeReservationRequestMessage.h"
#include "../messages/outgoing/payments/IntermediateNodeReservationResponseMessage.h"
#include "../messages/outgoing/payments/ReceiverInitPaymentRequestMessage.h"
#include "../messages/outgoing/payments/ReceiverInitPaymentResponseMessage.h"
#include "../messages/cycles/InBetweenNodeTopologyMessage.h"
#include "../messages/cycles/BoundaryNodeTopologyMessage.h"
#include "../messages/response/Response.h"
#include "../messages/response/RoutingTablesResponse.h"

#include "../../common/exceptions/ValueError.h"
#include "../../common/exceptions/ConflictError.h"

#include <boost/asio.hpp>
#include <boost/signals2.hpp>

#include <memory>
#include <vector>


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
