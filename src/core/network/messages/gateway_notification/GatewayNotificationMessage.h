#ifndef GEO_NETWORK_CLIENT_GATEWAYNOTIFICATIONMESSAGE_H
#define GEO_NETWORK_CLIENT_GATEWAYNOTIFICATIONMESSAGE_H

#include "../base/transaction/TransactionMessage.h"

class GatewayNotificationMessage : public TransactionMessage {

public:
    typedef shared_ptr<GatewayNotificationMessage> Shared;

public:
    using TransactionMessage::TransactionMessage;

    const MessageType typeID() const;
};


#endif //GEO_NETWORK_CLIENT_GATEWAYNOTIFICATIONMESSAGE_H
