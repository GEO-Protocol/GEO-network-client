#ifndef GEO_NETWORK_CLIENT_ROUTINGTABLESMESSAGE_H
#define GEO_NETWORK_CLIENT_ROUTINGTABLESMESSAGE_H

#include "Message.hpp"

#include "../../common/Types.h"
#include "../../common/memory/MemoryUtils.h"

#include "../../trust_lines/TrustLineUUID.h"
#include "../../transactions/transactions/base/TransactionUUID.h"

#include <memory>
#include <utility>
#include <cstdlib>
#include <stdint.h>

class RoutingTablesMessage : public Message {
public:
    typedef shared_ptr<RoutingTablesMessage> Shared;
    typedef uint64_t RecordsCount;


public:
    virtual const MessageType typeID() const = 0;

    const TransactionUUID &transactionUUID() const {

        throw NotImplementedError("RoutingTablesMessage: public Message::transactionUUID:"
                                      "Method not implemented.");
    }

    const TrustLineUUID &trustLineUUID() const {

        return mTrustLineUUID;
    }

    virtual pair<BytesShared, size_t> serializeToBytes() {

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
            mTrustLineUUID.data,
            NodeUUID::kBytesSize
        );
        //----------------------------------------------------
        return make_pair(
            dataBytesShared,
            bytesCount
        );
    }

protected:
    RoutingTablesMessage(){};

    RoutingTablesMessage(
        NodeUUID &senderUUID,
        TrustLineUUID &trustLineUUID) :

        Message(senderUUID),
        mTrustLineUUID(trustLineUUID){}

    virtual void deserializeFromBytes(
        BytesShared buffer) {

        Message::deserializeFromBytes(buffer);
        size_t bytesBufferOffset = Message::kOffsetToInheritedBytes();
        //----------------------------------------------------
        memcpy(
            mTrustLineUUID.data,
            buffer.get() + bytesBufferOffset,
            NodeUUID::kBytesSize
        );
    }

    static const size_t kOffsetToInheritedBytes() {

        static const size_t offset = Message::kOffsetToInheritedBytes() + TrustLineUUID::kBytesSize;
        return offset;
    }

protected:
    TrustLineUUID mTrustLineUUID;
};

#endif //GEO_NETWORK_CLIENT_ROUTINGTABLESMESSAGE_H
