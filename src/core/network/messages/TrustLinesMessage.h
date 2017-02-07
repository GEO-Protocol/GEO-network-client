#ifndef GEO_NETWORK_CLIENT_TRUSTLINESMESSAGE_H
#define GEO_NETWORK_CLIENT_TRUSTLINESMESSAGE_H

#include "Message.h"

#include "../../common/NodeUUID.h"
#include "../../transactions/TransactionUUID.h"

#include <memory>
#include <utility>
#include <stdint.h>
#include <malloc.h>

class TrustLinesMessage : public Message {
public:
    typedef shared_ptr<TrustLinesMessage> Shared;

public:
    const TransactionUUID &transactionUUID() const {
        return mTransactionUUID;
    }

protected:
    TrustLinesMessage() {};

    TrustLinesMessage(
        NodeUUID &senderUUID,
        TransactionUUID &transactionUUID) :

        Message(senderUUID),
        mTransactionUUID(transactionUUID) {};

    pair<ConstBytesShared, size_t> serializeParentToBytes() {
        size_t dataSize = sizeof(uint16_t) +
                          NodeUUID::kBytesSize +
                          TransactionUUID::kBytesSize;

        size_t dataBufferOffset = 0;

        byte *data = (byte *) calloc (dataSize, sizeof(byte));
        //----------------------------
        uint16_t type = typeID();
        memcpy(data, &type, sizeof(uint16_t));
        dataBufferOffset += sizeof(uint16_t);
        //----------------------------
        memcpy(data + dataBufferOffset, mSenderUUID.data, NodeUUID::kBytesSize);
        dataBufferOffset += NodeUUID::kBytesSize;
        //----------------------------
        memcpy(data + dataBufferOffset, mTransactionUUID.data, TransactionUUID::kBytesSize);
        //----------------------------
        return make_pair(ConstBytesShared(data, free), dataSize);
    }

    void deserializeParentFromBytes(
        byte *buffer) {
        uint16_t *typeID = new (buffer) uint16_t;
        size_t bytesBufferOffset = sizeof(uint16_t);
        //------------------------------
        memcpy(
            mSenderUUID.data,
            buffer + bytesBufferOffset,
            NodeUUID::kBytesSize
        );
        bytesBufferOffset += NodeUUID::kBytesSize;
        //------------------------------
        memcpy(
            mTransactionUUID.data,
            buffer + bytesBufferOffset,
            TransactionUUID::kBytesSize
        );
    }

    static const size_t kOffsetToInheritBytes() {
        static const size_t offset = sizeof(uint16_t) + NodeUUID::kBytesSize + TransactionUUID::kBytesSize;
        return offset;
    }

    virtual const MessageTypeID typeID() const = 0;


    const TrustLineUUID &trustLineUUID() const {
        throw NotImplementedError("TrustLinesMessage: public Message::trustLineUUID:"
                                      "Method not implemented.");
    }

    virtual pair<ConstBytesShared, size_t> serialize() = 0;

    virtual void deserialize(
        byte *buffer) = 0;

protected:
    const size_t kTrustLineAmountSize = 32;

    TransactionUUID mTransactionUUID;
};

#endif //GEO_NETWORK_CLIENT_TRUSTLINESMESSAGE_H
