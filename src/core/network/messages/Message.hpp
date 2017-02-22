#ifndef GEO_NETWORK_CLIENT_MESSAGE_H
#define GEO_NETWORK_CLIENT_MESSAGE_H


#include "../../common/Types.h"
#include "../../common/NodeUUID.h"
#include "../../common/memory/MemoryUtils.h"

// TODO: remove this imports
#include "../../common/exceptions/Exception.h"
#include "../../trust_lines/TrustLineUUID.h"
#include "../../transactions/transactions/base/TransactionUUID.h"


using namespace std;


class Message {
public:
    typedef shared_ptr<Message> Shared;

public:
    // TODO: cut "..MessageType" from all enum records
    enum MessageTypeID {
        OpenTrustLineMessageType = 1,
        AcceptTrustLineMessageType = 2,
        SetTrustLineMessageType = 3,
        CloseTrustLineMessageType = 4,
        RejectTrustLineMessageType = 5,
        UpdateTrustLineMessageType = 6,
        FirstLevelRoutingTableOutgoingMessageType = 7,
        FirstLevelRoutingTableIncomingMessageType = 8,
        SecondLevelRoutingTableOutgoingMessageType = 9,
        SecondLevelRoutingTableIncomingMessageType = 10,

        Payments_ReceiverInitPayment,
        Payments_ReceiverApprove,

        ResponseMessageType = 1000
    };

    // TODO: (DM) rename to "SerializedMessageType"
    typedef uint16_t MessageType;

public:
    virtual const MessageType typeID() const = 0;

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

    //TODO:: move into another message
    const NodeUUID &senderUUID() const {
        return mSenderUUID;
    }

    //TODO:: move into another message
    virtual const TransactionUUID &transactionUUID() const = 0;

    //TODO:: move into another message
    virtual const TrustLineUUID &trustLineUUID() const = 0;

    /**
     *
     * @throws bad_alloc;
     */
    virtual pair<BytesShared, size_t> serializeToBytes() {

        size_t bytesCount =
            + sizeof(MessageType)
            + NodeUUID::kBytesSize;

        BytesShared dataBytesShared = tryMalloc(bytesCount);
        size_t dataBytesOffset = 0;
        //-----------------------------------------------------
        MessageType type = typeID();
        memcpy(
            dataBytesShared.get(),
            &type,
            sizeof(MessageType)
        );
        dataBytesOffset += sizeof(MessageType);
        //-----------------------------------------------------
        memcpy(
            dataBytesShared.get() + dataBytesOffset,
            mSenderUUID.data,
            NodeUUID::kBytesSize
        );
        //-----------------------------------------------------
        return make_pair(
            dataBytesShared,
            bytesCount
        );
    }

protected:
    // TODO: (DM) empty constructor is bad by design. Try to remove it.
    Message() {};

    Message(
        NodeUUID &senderUUID) :
        mSenderUUID(senderUUID){}

    virtual void deserializeFromBytes(
        BytesShared buffer){

        size_t bytesBufferOffset = 0;

        MessageType *messageType = new (buffer.get()) MessageType;
        bytesBufferOffset += sizeof(MessageType);
        //-----------------------------------------------------
        memcpy(
          mSenderUUID.data,
          buffer.get() + bytesBufferOffset,
          NodeUUID::kBytesSize
        );
    }

    static const size_t kOffsetToInheritedBytes(){

        static const size_t offset = sizeof(MessageType) + NodeUUID::kBytesSize;
        return offset;
    }

protected:
    NodeUUID mSenderUUID;
};

#endif //GEO_NETWORK_CLIENT_MESSAGE_H
