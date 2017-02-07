#ifndef GEO_NETWORK_CLIENT_ROUTINGTABLESMESSAGE_H
#define GEO_NETWORK_CLIENT_ROUTINGTABLESMESSAGE_H

#include "Message.h"

#include "../../trust_lines/TrustLineUUID.h"

#include <memory>
#include <utility>
#include <stdint.h>
#include <malloc.h>

class RoutingTablesMessage : public Message {
public:
    typedef shared_ptr<RoutingTablesMessage> Shared;

protected:
    RoutingTablesMessage(){};

    RoutingTablesMessage(
        NodeUUID &senderUUID,
        NodeUUID &contractorUUID,
        TrustLineUUID &trustLineUUID) :

        Message(senderUUID),
        mContractorUUID(contractorUUID),
        mTrustLineUUID(trustLineUUID){}

    pair<ConstBytesShared, size_t> serializeParentToBytes() {
        size_t dataSize = sizeof(uint16_t) +
            NodeUUID::kBytesSize +
            NodeUUID::kBytesSize +
            TrustLineUUID::kBytesSize;

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
        memcpy(data + dataBufferOffset, mContractorUUID.data, NodeUUID::kBytesSize);
        dataBufferOffset += NodeUUID::kBytesSize;
        //----------------------------
        memcpy(data + dataBufferOffset, mTrustLineUUID.data, TrustLineUUID::kBytesSize);
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
            mContractorUUID.data,
            buffer + bytesBufferOffset,
            NodeUUID::kBytesSize
        );
        bytesBufferOffset += NodeUUID::kBytesSize + NodeUUID::kBytesSize;
        //------------------------------
        memcpy(
            mTrustLineUUID.data,
            buffer + bytesBufferOffset,
            TrustLineUUID::kBytesSize
        );
    }

    static const size_t kOffsetToInheritBytes() {
        static const size_t offset = sizeof(uint16_t) + NodeUUID::kBytesSize + NodeUUID::kBytesSize + TrustLineUUID::kBytesSize;
        return offset;
    }

    virtual const MessageTypeID typeID() const = 0;

    const TransactionUUID &transactionUUID() const {
        throw NotImplementedError("RoutingTablesMessage: public Message::UUID:"
                                      "Method not implemented.");
    }

    const TrustLineUUID &trustLineUUID() const {
        return mTrustLineUUID;
    }

    virtual pair<ConstBytesShared, size_t> serialize() = 0;

    virtual void deserialize(
        byte *buffer) = 0;

protected:
    NodeUUID mContractorUUID;
    TrustLineUUID mTrustLineUUID;
};

#endif //GEO_NETWORK_CLIENT_ROUTINGTABLESMESSAGE_H
