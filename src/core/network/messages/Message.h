#ifndef GEO_NETWORK_CLIENT_MESSAGE_H
#define GEO_NETWORK_CLIENT_MESSAGE_H

#include "../../common/Types.h"
#include "../../common/NodeUUID.h"
#include "../../transactions/TransactionUUID.h"

#include "../../common/exceptions/Exception.h"
#include "../../common/exceptions/MemoryError.h"

#include <boost/crc.hpp>

#include <stdint.h>
#include <malloc.h>
#include <vector>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <utility>


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
        ResponseMessageType = 11
    };

public:
    const NodeUUID &senderUUID() const { return mSenderUUID; }

    const TransactionUUID &transactionUUID() const { return mTransactionUUID; }

    virtual pair<ConstBytesShared, size_t> serialize() = 0;

    virtual const MessageTypeID typeID() const = 0;

protected:
    Message() {};

    Message(NodeUUID &sender, TransactionUUID &transactionUUID) : mSenderUUID(sender), mTransactionUUID(transactionUUID) {};

    virtual void deserialize(
        byte *buffer) = 0;

protected:
    NodeUUID mSenderUUID;
    TransactionUUID mTransactionUUID;
};

#endif //GEO_NETWORK_CLIENT_MESSAGE_H
