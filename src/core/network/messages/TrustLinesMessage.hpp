#ifndef GEO_NETWORK_CLIENT_TRUSTLINESMESSAGE_H
#define GEO_NETWORK_CLIENT_TRUSTLINESMESSAGE_H

#include "Message.hpp"
#include "result/MessageResult.h"

#include "../../common/Types.h"
#include "../../common/memory/MemoryUtils.h"

#include "../../common/NodeUUID.h"
#include "../../trust_lines/TrustLineUUID.h"
#include "../../transactions/TransactionUUID.h"

#include <memory>
#include <utility>
#include <cstdlib>
#include <stdint.h>

using namespace std;

class TrustLinesMessage : public Message {
public:
    typedef shared_ptr<TrustLinesMessage> Shared;

public:
    virtual const MessageType typeID() const = 0;

    const TransactionUUID &transactionUUID() const {

        return mTransactionUUID;
    }

    const TrustLineUUID &trustLineUUID() const {

        throw NotImplementedError("TrustLinesMessage: public Message::trustLineUUID:"
                                      "Method not implemented.");
    }

    virtual pair<BytesShared, size_t> serializeToBytes(){

        auto parentBytesAndCount = Message::serializeToBytes();
        size_t bytesCount = parentBytesAndCount.second +
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
    TrustLinesMessage() {};

    TrustLinesMessage(
        NodeUUID &senderUUID,
        TransactionUUID &transactionUUID) :

        Message(senderUUID),
        mTransactionUUID(transactionUUID) {};

    virtual void deserializeFromBytes(
        BytesShared buffer){

        Message::deserializeFromBytes(buffer);
        size_t bytesBufferOffset = Message::kOffsetToInheritedBytes();
        //----------------------------------------------------
        memcpy(
            mTransactionUUID.data,
            buffer.get() + bytesBufferOffset,
            TransactionUUID::kBytesSize
        );
    }

    static const size_t kOffsetToInheritedBytes() {

        static const size_t offset = Message::kOffsetToInheritedBytes() + TransactionUUID::kBytesSize;
        return offset;
    }

protected:
    TransactionUUID mTransactionUUID;
};

#endif //GEO_NETWORK_CLIENT_TRUSTLINESMESSAGE_H
