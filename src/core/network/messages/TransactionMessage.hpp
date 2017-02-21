#ifndef GEO_NETWORK_CLIENT_TRANSACTIONMESSAGE_H
#define GEO_NETWORK_CLIENT_TRANSACTIONMESSAGE_H

#include "Message.hpp"

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

class TransactionMessage:
    public Message { // TODO: change to SenderMessage

public:
    typedef shared_ptr<TransactionMessage> Shared;
    typedef shared_ptr<const TransactionMessage> ConstShared;

public:
    virtual const MessageType typeID() const = 0;

    const TransactionUUID &transactionUUID() const {

        return mTransactionUUID;
    }

    const TrustLineUUID &trustLineUUID() const {

        throw NotImplementedError("TransactionMessage: public Message::trustLineUUID:"
                                      "Method not implemented.");
    }

    /*!
     *
     * Throws bad_alloc;
     */
    virtual pair<BytesShared, size_t> serializeToBytes() {

        auto parentBytesAndCount = Message::serializeToBytes();
        size_t bytesCount =
            + parentBytesAndCount.second
            + TransactionUUID::kBytesSize;
        BytesShared dataBytesShared = tryMalloc(bytesCount);
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

protected:
    TransactionMessage() {};

    TransactionMessage(
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

#endif //GEO_NETWORK_CLIENT_TRANSACTIONMESSAGE_H
