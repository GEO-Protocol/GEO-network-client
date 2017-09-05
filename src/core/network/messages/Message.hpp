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
    // it used in pair of BasePaymentTransaction::PathID
    // so if you change this one, you should change another too
    typedef uint16_t PathID;

public:
    enum MessageType {
        /*
         * System messages types
         */
        System_Confirmation = 0,

        /*
         * Trust lines
         */
        TrustLines_SetIncoming = 100,

        /*
         * Payments messages
         */
        Payments_ReceiverInitPaymentRequest = 201,
        Payments_ReceiverInitPaymentResponse = 202,
        Payments_CoordinatorReservationRequest = 203,
        Payments_CoordinatorReservationResponse = 204,
        Payments_IntermediateNodeReservationRequest = 205,
        Payments_IntermediateNodeReservationResponse = 206,

        Payments_CoordinatorCycleReservationRequest = 207,
        Payments_CoordinatorCycleReservationResponse = 208,
        Payments_IntermediateNodeCycleReservationRequest = 209,
        Payments_IntermediateNodeCycleReservationResponse = 210,

        Payments_FinalAmountsConfiguration = 211,
        Payments_FinalAmountsConfigurationResponse = 212,

        Payments_ParticipantsVotes = 213,
        Payments_VotesStatusRequest = 214,
        Payments_FinalPathConfiguration = 215,
        Payments_FinalPathCycleConfiguration = 216,
        Payments_TTLProlongationRequest = 217,
        Payments_TTLProlongationResponse = 218,

        /*
         * Cycles
         */
        Cycles_ThreeNodesBalancesRequest = 300,
        Cycles_ThreeNodesBalancesResponse = 301,
        Cycles_FourNodesBalancesRequest = 302,
        Cycles_FourNodesBalancesResponse = 303,
        Cycles_FiveNodesBoundary = 304,
        Cycles_FiveNodesMiddleware = 305,
        Cycles_SixNodesBoundary = 306,
        Cycles_SixNodesMiddleware = 307,

        /*
         * Max flow
         */
        MaxFlow_InitiateCalculation = 400,
        MaxFlow_CalculationSourceFirstLevel = 401,
        MaxFlow_CalculationTargetFirstLevel = 402,
        MaxFlow_CalculationSourceSecondLevel = 403,
        MaxFlow_CalculationTargetSecondLevel = 404,
        MaxFlow_ResultMaxFlowCalculation = 405,

        /*
         * Total balance
         */
        TotalBalance_Request  = 500,
        TotalBalance_Response = 501,

        // ToDo: remove this
        ResponseMessageType = 1000,

        /*
         * DEBUG
         */
        // Obvious, that we have to set this code
        Debug = 6666,
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
