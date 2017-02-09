#ifndef GEO_NETWORK_CLIENT_TRANSACTIONMESSAGE_H
#define GEO_NETWORK_CLIENT_TRANSACTIONMESSAGE_H

#include "Message.hpp"

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

class TransactionMessage:
    public Message { // todo: change to message with sender

public:
    typedef shared_ptr<TransactionMessage> Shared;
    typedef shared_ptr<const TransactionMessage> ConstShared;

public:
    virtual const MessageType typeID() const = 0;

    const TransactionUUID &transactionUUID() const {
        return mTransactionUUID;
    }

    /*!
     *
     * Throws bad_alloc;
     */
    virtual pair<BytesShared, size_t> serializeToBytes() const {
        auto parentBytesAndCount = Message::serializeToBytes();

        size_t bytesCount =
            + parentBytesAndCount.second
            + TransactionUUID::kBytesSize;

        BytesShared buffer =
            tryMalloc(
                bytesCount);

        auto initialOffset = buffer.get();
        memcpy(
            initialOffset,
            parentBytesAndCount.first.get(),
            parentBytesAndCount.second);

        auto transactionUUIDOffset =
            initialOffset
            + parentBytesAndCount.second;
        memcpy(
            transactionUUIDOffset,
            mTransactionUUID.data,
            TransactionUUID::kBytesSize);

        return make_pair(
            buffer,
            bytesCount);
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
