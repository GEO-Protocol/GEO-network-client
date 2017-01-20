#ifndef GEO_NETWORK_CLIENT_MESSAGE_H
#define GEO_NETWORK_CLIENT_MESSAGE_H

#include "../../common/Types.h"
#include "../../common/NodeUUID.h"
#include "../../transactions/TransactionUUID.h"

#include "../channels/packet/PacketHeader.h"
#include "../channels/packet/Packet.h"

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
        CloseTrustLineMessageType = 3,
        RejectTrustLineMessageType = 4,
        ResponseMessageType = 5
    };

public:
    Message() {};

    Message(NodeUUID &sender, TransactionUUID &transactionUUID) : mSenderUUID(sender), mTransactionUUID(transactionUUID) {};

    const NodeUUID &senderUUID() const { return mSenderUUID; }

    const TransactionUUID &transactionUUID() const { return mTransactionUUID; }

    virtual pair<ConstBytesShared, size_t> serialize() = 0;

    virtual void deserialize(
        byte *buffer) = 0;

    virtual const MessageTypeID typeID() const = 0;

protected:
    NodeUUID mSenderUUID;
    TransactionUUID mTransactionUUID;
};

#endif //GEO_NETWORK_CLIENT_MESSAGE_H
