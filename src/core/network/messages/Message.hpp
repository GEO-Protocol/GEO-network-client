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
    enum MessageType {
        /*
         * System messages types
         */
        System_Confirmation = 0,

        /*
         * Trust lines
         */
        TrustLines_SetIncoming = 101,
        TrustLines_SetIncomingInitial = 102,
        TrustLines_CloseOutgoing = 103,
        TrustLines_Confirmation = 104,
        TrustLines_PublicKey = 105,
        TrustLines_HashConfirmation = 106,
        TrustLines_Audit = 107,
        TrustLines_AuditConfirmation = 108,

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

        Payments_FinalPathConfiguration = 213,
        Payments_FinalPathCycleConfiguration = 214,

        Payments_TTLProlongationRequest = 215,
        Payments_TTLProlongationResponse = 216,

        Payments_TransactionPublicKeyHash = 217,
        Payments_ParticipantVote = 218,
        Payments_ParticipantsPublicKeys = 219,
        Payments_ParticipantsVotes = 220,

        Payments_VotesStatusRequest = 221,

        /*
         * Cycles
         */
        Cycles_ThreeNodesBalancesRequest = 300,
        Cycles_ThreeNodesBalancesResponse = 301,
        Cycles_FourNodesNegativeBalanceRequest = 302,
        Cycles_FourNodesPositiveBalanceRequest = 303,
        Cycles_FourNodesBalancesResponse = 304,
        Cycles_FiveNodesBoundary = 305,
        Cycles_FiveNodesMiddleware = 306,
        Cycles_SixNodesBoundary = 307,
        Cycles_SixNodesMiddleware = 308,

        /*
         * Max flow
         */
        MaxFlow_InitiateCalculation = 400,
        MaxFlow_CalculationSourceFirstLevel = 401,
        MaxFlow_CalculationTargetFirstLevel = 402,
        MaxFlow_CalculationSourceSecondLevel = 403,
        MaxFlow_CalculationTargetSecondLevel = 404,
        MaxFlow_ResultMaxFlowCalculation = 405,
        MaxFlow_ResultMaxFlowCalculationFromGateway = 406,
        MaxFlow_Confirmation = 407,

        /*
         * Empty slot with codes 500-599
         */

        /*
         * Empty slot with codes 600-699
         */

        /*
         * Gateway notification & Routing tables
         */
        GatewayNotification = 700,
        RoutingTableResponse = 701,

        NoEquivalent = 800,

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

    virtual const bool isDestinationMessage() const
    {
        return false;
    }

    virtual const bool isAddToConfirmationRequiredMessagesHandler() const
    {
        return false;
    }

    virtual const bool isAddToConfirmationNotStronglyRequiredMessagesHandler() const
    {
        return false;
    }

    virtual const bool isCheckCachedResponse() const
    {
        return false;
    }

    virtual const SerializedEquivalent equivalent() const
    {
        return 0;
    }

    virtual const MessageType typeID() const = 0;

    /**
     * @throws bad_alloc;
     */
    virtual pair<BytesShared, size_t> serializeToBytes() const
        noexcept(false)
    {
        const SerializedType kMessageType = typeID();
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
    virtual const size_t kOffsetToInheritedBytes() const
    {
        return sizeof(SerializedType);
    }
};

#endif //GEO_NETWORK_CLIENT_MESSAGE_H
