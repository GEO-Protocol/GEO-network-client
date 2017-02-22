#ifndef GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONMESSAGE_H
#define GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONMESSAGE_H

#include "Message.hpp"
#include "result/MessageResult.h"

#include "../../common/Types.h"
#include "../../common/memory/MemoryUtils.h"

#include "../../common/NodeUUID.h"
#include "../../trust_lines/TrustLineUUID.h"
#include "../../transactions/transactions/base/TransactionUUID.h"

#include <memory>
#include <utility>
#include <cstdlib>
#include <stdint.h>

using namespace std;

class MaxFlowCalculationMessage : public Message {
public:
    typedef shared_ptr<MaxFlowCalculationMessage> Shared;

public:
    virtual const MessageType typeID() const = 0;

    const NodeUUID &targetUUID() const {

        return mTargetUUID;
    }

    const TransactionUUID &transactionUUID() const {

        return mTransactionUUID;
    }

    const TrustLineUUID &trustLineUUID() const {

        throw NotImplementedError("MaxFlowCalculationMessage: public Message::trustLineUUID:"
                                      "Method not implemented.");
    }

    virtual pair<BytesShared, size_t> serializeToBytes() {

        auto parentBytesAndCount = Message::serializeToBytes();
        size_t bytesCount = parentBytesAndCount.second +
                            NodeUUID::kBytesSize +
                            TransactionUUID::kBytesSize;
        BytesShared dataBytesShared = tryCalloc(bytesCount);
        size_t dataBytesOffset = 0;
        //----------------------------------------------------
        memcpy(
            dataBytesShared.get(),
            parentBytesAndCount.first.get(),
            parentBytesAndCount.second
        );
        dataBytesOffset += parentBytesAndCount.second;
        //----------------------------------------------------
        memcpy(
            dataBytesShared.get() + dataBytesOffset,
            mTargetUUID.data,
            NodeUUID::kBytesSize
        );
        dataBytesOffset += NodeUUID::kBytesSize;
        //----------------------------------------------------
        memcpy(
            dataBytesShared.get() + dataBytesOffset,
            mTransactionUUID.data,
            TransactionUUID::kBytesSize
        );
        //----------------------------------------------------
        return make_pair(
            dataBytesShared,
            bytesCount
        );
    }

    MessageResult::SharedConst customCodeResult(
        uint16_t code) const {

        return MessageResult::SharedConst(
            new MessageResult(
                mSenderUUID,
                mTransactionUUID,
                code)
        );
    }

protected:

    MaxFlowCalculationMessage(){}

    MaxFlowCalculationMessage(
        NodeUUID &senderUUID,
        NodeUUID &targetUUID,
        TransactionUUID &transactionUUID) :

        Message(senderUUID),
        mTargetUUID(targetUUID),
        mTransactionUUID(transactionUUID) {};

    virtual void deserializeFromBytes(
        BytesShared buffer){

        Message::deserializeFromBytes(buffer);
        size_t bytesBufferOffset = Message::kOffsetToInheritedBytes();
        //----------------------------------------------------
        memcpy(
            mTargetUUID.data,
            buffer.get() + bytesBufferOffset,
            NodeUUID::kBytesSize
        );
        bytesBufferOffset += NodeUUID::kBytesSize;
        //----------------------------------------------------
        memcpy(
            mTransactionUUID.data,
            buffer.get() + bytesBufferOffset,
            TransactionUUID::kBytesSize
        );
    }

    static const size_t kOffsetToInheritedBytes() {

        static const size_t offset = Message::kOffsetToInheritedBytes()
                                     + NodeUUID::kBytesSize + TransactionUUID::kBytesSize;
        return offset;
    }

protected:
    TransactionUUID mTransactionUUID;
    NodeUUID mTargetUUID;
};

#endif //GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONMESSAGE_H
