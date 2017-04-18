#ifndef GEO_NETWORK_CLIENT_REQUESTROUTINGTABLESMESSAGE_H
#define GEO_NETWORK_CLIENT_REQUESTROUTINGTABLESMESSAGE_H

#include "../base/transaction/TransactionMessage.h"


class RequestRoutingTablesMessage:
    public TransactionMessage {

public:
    typedef shared_ptr<RequestRoutingTablesMessage> Shared;

public:
    using TransactionMessage::TransactionMessage;

    const MessageType typeID() const
        noexcept;
};


#endif //GEO_NETWORK_CLIENT_REQUESTROUTINGTABLESMESSAGE_H
