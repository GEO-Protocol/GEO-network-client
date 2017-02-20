#ifndef GEO_NETWORK_CLIENT_MESSAGE_H
#define GEO_NETWORK_CLIENT_MESSAGE_H

#include "../../common/Types.h"

#include "../../common/exceptions/Exception.h"

#include <memory>
#include <utility>
#include <stdint.h>

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
        AcceptTrustLineMessageType,
        SetTrustLineMessageType,
        CloseTrustLineMessageType,
        RejectTrustLineMessageType,
        UpdateTrustLineMessageType,
        FirstLevelRoutingTableOutgoingMessageType,
        FirstLevelRoutingTableIncomingMessageType,
        SecondLevelRoutingTableOutgoingMessageType,
        SecondLevelRoutingTableIncomingMessageType,
        ReceiverInitPaymentMessageType,
        OperationStateMessageType,
        ResponseMessageType = 1000,
        RoutingTablesResponseMessageType
    };
    typedef uint16_t MessageType;

public:
    virtual const MessageType typeID() const = 0;

    virtual pair<BytesShared, size_t> serializeToBytes() = 0;

protected:
    Message() {};

    virtual void deserializeFromBytes(
        BytesShared buffer) = 0;
};
#endif //GEO_NETWORK_CLIENT_MESSAGE_H
