#ifndef GEO_NETWORK_CLIENT_REQUESTROUTINGTABLESMESSAGE_H
#define GEO_NETWORK_CLIENT_REQUESTROUTINGTABLESMESSAGE_H

#include "../base/transaction/TransactionMessage.h"

class RequestRoutingTablesMessage : public TransactionMessage {

public:
    typedef shared_ptr<RequestRoutingTablesMessage> Shared;

public:

    RequestRoutingTablesMessage(
        const NodeUUID& senderUUID,
        const TransactionUUID &transactionUUID);

    RequestRoutingTablesMessage(
        BytesShared buffer);

    const MessageType typeID() const;

};


#endif //GEO_NETWORK_CLIENT_REQUESTROUTINGTABLESMESSAGE_H
