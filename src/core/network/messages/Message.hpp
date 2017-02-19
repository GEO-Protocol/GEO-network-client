#ifndef GEO_NETWORK_CLIENT_MESSAGE_H
#define GEO_NETWORK_CLIENT_MESSAGE_H

#include "../../common/Types.h"
#include "../../common/memory/MemoryUtils.h"

#include "../../common/NodeUUID.h"
#include "../../trust_lines/TrustLineUUID.h"
#include "../../transactions/transactions/base/TransactionUUID.h"

#include "../../common/exceptions/Exception.h"

#include <memory>
#include <utility>
#include <cstdlib>
#include <stdint.h>

using namespace std;

class NotImplementedError : public Exception {
    using Exception::Exception;
};

class Message {
public:
    typedef shared_ptr<Message> Shared;

public:
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

        ResponseMessageType = 1000
    };
    typedef uint16_t MessageType;

public:
    virtual const MessageType typeID() const = 0;

    const NodeUUID &senderUUID() const {

        return mSenderUUID;
    }

    //TODO:: move into another message
    virtual const TransactionUUID &transactionUUID() const = 0;

    //TODO:: move into another message
    virtual const TrustLineUUID &trustLineUUID() const = 0;

    virtual pair<BytesShared, size_t> serializeToBytes() {

        size_t bytesCount = sizeof(MessageType) +
            NodeUUID::kBytesSize;
        BytesShared dataBytesShared = tryCalloc(bytesCount);
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
