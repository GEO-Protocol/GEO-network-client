#ifndef GEO_NETWORK_CLIENT_MESSAGE_H
#define GEO_NETWORK_CLIENT_MESSAGE_H

#include "../../common/Types.h"
#include "../../common/NodeUUID.h"
#include "../../trust_lines/TrustLineUUID.h"
#include "../../transactions/TransactionUUID.h"

#include "../../common/exceptions/Exception.h"
#include "../../common/exceptions/MemoryError.h"

#include <memory>
#include <utility>
#include <stdint.h>
#include <cstdlib>

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
    virtual const MessageTypeID typeID() const = 0;

    const NodeUUID &senderUUID() const{
        return mSenderUUID;
    }

    virtual const TransactionUUID &transactionUUID() const = 0;

    virtual const TrustLineUUID &trustLineUUID() const = 0;

    virtual pair<ConstBytesShared, size_t> serialize() = 0;

protected:
    Message() {};

    Message(
        NodeUUID &senderUUID) :

        mSenderUUID(senderUUID){}

    virtual void deserialize(
        byte *buffer) = 0;

protected:
    NodeUUID mSenderUUID;

};

#endif //GEO_NETWORK_CLIENT_MESSAGE_H
