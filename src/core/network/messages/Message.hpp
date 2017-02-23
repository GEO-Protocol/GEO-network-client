#ifndef GEO_NETWORK_CLIENT_MESSAGE_H
#define GEO_NETWORK_CLIENT_MESSAGE_H

#include "../../common/Types.h"

#include "../../common/exceptions/Exception.h"

#include <memory>
#include <utility>
#include <stdint.h>

using namespace std;

class NotImplementedError : public Exception {
    using Exception::Exception;
};

class Message {
public:
    typedef shared_ptr<Message> Shared;

public:
    // TODO: cut "..MessageType" from all enum records
    enum MessageTypeID {
        OpenTrustLineMessageType = 1,
        AcceptTrustLineMessageType,
        SetTrustLineMessageType,
        CloseTrustLineMessageType,
        RejectTrustLineMessageType,
        UpdateTrustLineMessageType,
        FirstLevelRoutingTableOutgoingMessageType,
        FirstLevelRoutingTableIncomingMessageType,
        SecondLevelRoutingTableOutgoingMessageType,
        SecondLevelRoutingTableIncomingMessageType,
        ReceiverInitPaymentMessageType,
        OperationStateMessageType,
        InitiateMaxFlowCalculationMessageType,
        ReceiveMaxFlowCalculationOnTargetMessageType,
        ResultMaxFlowCalculationFromTargetMessageType,
        SendResultMaxFlowCalculationFromTargetMessageType,
        SendMaxFlowCalculationSourceFstLevelMessageType,
        MaxFlowCalculationSourceFstLevelInMessageType,
        SendMaxFlowCalculationTargetFstLevelMessageType,
        MaxFlowCalculationTargetFstLevelInMessageType,
        MaxFlowCalculationSourceFstLevelOutMessageType,
        MaxFlowCalculationSourceSndLevelInMessageType,
        MaxFlowCalculationTargetFstLevelOutMessageType,
        MaxFlowCalculationTargetSndLevelInMessageType,
        SendResultMaxFlowCalculationFromSourceMessageType,
        ResultMaxFlowCalculationFromSourceMessageType,

        ResponseMessageType = 1000,
        RoutingTablesResponseMessageType
    };
    // TODO: (DM) rename to "SerializedMessageType"
    typedef uint16_t MessageType;

public:
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
    virtual const bool isTransactionMessage() const {

        return false;
    }
    virtual const bool isRoutingTableResponseMessage() const {

        return false;
    }
    virtual const bool isMaxFlowCalculationResponseMessage() const {

        return false;
    }
    virtual const bool isCyclesDiscoveringResponseMessage() const {

        return false;
    }

    virtual const MessageType typeID() const = 0;

    virtual pair<BytesShared, size_t> serializeToBytes() = 0;

protected:
    Message() {};

    virtual void deserializeFromBytes(
        BytesShared buffer) = 0;
};

#endif //GEO_NETWORK_CLIENT_MESSAGE_H
