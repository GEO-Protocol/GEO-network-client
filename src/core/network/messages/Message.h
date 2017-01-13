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
    using Exception :: Exception;
};

class Message {

public:
    typedef shared_ptr<Message> Shared;

public:
    enum MessageTypeID {
        OpenTrustLineMessageType = 1,
        AcceptTrustLineMessageType = 2
    };

public:
    Message() {};

    Message(TransactionUUID transactionUUID) : mTransactionUUID(transactionUUID) {};

    Message(NodeUUID sender) : mSender(sender) {};

    virtual pair<ConstBytesShared, size_t> serialize() = 0;

    virtual void deserialize(
        byte* buffer) = 0;

    virtual const MessageTypeID typeID() const = 0;

protected:
    NodeUUID mSender;
    TransactionUUID mTransactionUUID;
};

#endif //GEO_NETWORK_CLIENT_MESSAGE_H
