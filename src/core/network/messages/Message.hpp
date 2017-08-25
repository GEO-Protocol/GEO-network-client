#ifndef GEO_NETWORK_CLIENT_MESSAGE_H
#define GEO_NETWORK_CLIENT_MESSAGE_H

#include "../../common/memory/MemoryUtils.h"
#include "../communicator/internal/common/Packet.hpp"

#include <limits>


using namespace std;


class Message {
public:
    typedef shared_ptr<Message> Shared;
    typedef uint16_t SerializedType;

public:
    // TODO: move it into separate *.h file.
    typedef uint16_t PathUUID;

public:
    enum MessageType {
        /*
         * System messages types
         */
        System_Confirmation = 0,

        /*
         * Trust lines
         */
        TrustLines_SetIncoming,

        /*
         * Payments messages
         */
        Payments_ReceiverInitPaymentRequest,
        Payments_ReceiverInitPaymentResponse,
        Payments_CoordinatorReservationRequest,
        Payments_CoordinatorReservationResponse,
        Payments_IntermediateNodeReservationRequest,
        Payments_IntermediateNodeReservationResponse,

        Payments_CoordinatorCycleReservationRequest,
        Payments_CoordinatorCycleReservationResponse,
        Payments_IntermediateNodeCycleReservationRequest,
        Payments_IntermediateNodeCycleReservationResponse,

        Payments_ParticipantsVotes,
        Payments_VotesStatusRequest,
        Payments_FinalPathConfiguration,
        Payments_FinalPathCycleConfiguration,
        Payments_TTLProlongation,

        /*
         * Cycles
         */
        Cycles_ThreeNodesBalancesRequest,
        Cycles_ThreeNodesBalancesResponse,
        Cycles_FourNodesBalancesRequest,
        Cycles_FourNodesBalancesResponse,
        Cycles_FiveNodesBoundary,
        Cycles_FiveNodesMiddleware,
        Cycles_SixNodesBoundary,
        Cycles_SixNodesMiddleware,

        /*
         * Routing tables messages
         */
        RoutingTables_NeighborsRequest,
        RoutingTables_NeighborsResponse,
        RoutingTables_NotificationTrustLineCreated,
        RoutingTables_NotificationTrustLineRemoved,
        RoutingTables_CRC32Rt2RequestMessage,
        RoutingTables_CRC32Rt2ResponseMessage,
        RoutingTables_CRC32Rt2ThirdLevelResponseMessage,

        /*
         * Paths
         */
        Paths_RequestRoutingTables,
        Paths_ResultRoutingTableFirstLevel,
        Paths_ResultRoutingTableSecondLevel,
        Paths_ResultRoutingTableThirdLevel,

        /*
         * Max flow
         */
        MaxFlow_InitiateCalculation,
        MaxFlow_CalculationSourceFirstLevel,
        MaxFlow_CalculationTargetFirstLevel,
        MaxFlow_CalculationSourceSecondLevel,
        MaxFlow_CalculationTargetSecondLevel,
        MaxFlow_ResultMaxFlowCalculation,

        /*
         * Total balance
         */
        TotalBalance_Request,
        TotalBalance_Response,

        // ToDo: remove this
        ResponseMessageType = 1000,

        /*
         * DEBUG
         */
        Debug,
    };

public:
    virtual ~Message() = default;

    /*
     * Returns max allowed size of the message in bytes.
     */
    static size_t maxSize()
    {
        return
             numeric_limits<PacketHeader::PacketIndex>::max() * Packet::kMaxSize -
            (numeric_limits<PacketHeader::PacketIndex>::max() * PacketHeader::kSize);
    }

    /*
     * Base "Message" is abstract.
     * Some of it's derived classes are used for various transactions responses.
     *
     * Transactions scheduler requires mechanism to know
     * which response to attach to which transaction.
     * The simplest way to do this - to attach response to the transaction by it's UUID
     * (scheduler checks if transactionUUID of the response is uqual to the transaction).
     *
     * But transaction UUID may be redundant is ome cases
     * (for example, in routing table responses,
     * max flow calculation responses, and several other)
     *
     * This methods set makes it possible for the transactions scheduler to know,
     * how to decide which response should be attached to which transaction,
     * and implement custom attach logic for each one response type.
     *
     *
     * Derived classes of specific responses must override one of this methods.
     */
    virtual const bool isTransactionMessage() const
    {
        return false;
    }

    virtual const bool isRoutingTableMessage() const
    {
        return false;
    }

    virtual const bool isRoutingTableResponseMessage() const
    {
        return false;
    }

    virtual const bool isMaxFlowCalculationResponseMessage() const
    {
        return false;
    }

    virtual const bool isCyclesDiscoveringResponseMessage() const
    {
        return false;
    }


    virtual const MessageType typeID() const = 0;

    /**
     * @throws bad_alloc;
     */
    virtual pair<BytesShared, size_t> serializeToBytes() const
        noexcept(false)
    {
        const uint16_t kMessageType = typeID();
        auto buffer = tryMalloc(sizeof(kMessageType));

        memcpy(
            buffer.get(),
            &kMessageType,
            sizeof(kMessageType));

        return make_pair(
            buffer,
            sizeof(kMessageType));
    }

protected:
    virtual void deserializeFromBytes(
        BytesShared buffer)
    {}

    virtual const size_t kOffsetToInheritedBytes() const
    {
        return sizeof(uint16_t);
    }
};

#endif //GEO_NETWORK_CLIENT_MESSAGE_H
