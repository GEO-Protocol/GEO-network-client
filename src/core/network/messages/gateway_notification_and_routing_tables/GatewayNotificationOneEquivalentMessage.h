#ifndef GEO_NETWORK_CLIENT_GATEWAYNOTIFICATIONONEEQUIVALENTMESSAGE_H
#define GEO_NETWORK_CLIENT_GATEWAYNOTIFICATIONONEEQUIVALENTMESSAGE_H

#include "../base/transaction/TransactionMessage.h"

class GatewayNotificationOneEquivalentMessage : public TransactionMessage {

public:
    typedef shared_ptr<GatewayNotificationOneEquivalentMessage> Shared;

public:
    using TransactionMessage::TransactionMessage;

    const MessageType typeID() const;

    const bool isAddToConfirmationRequiredMessagesHandler() const;
};


#endif //GEO_NETWORK_CLIENT_GATEWAYNOTIFICATIONONEEQUIVALENTMESSAGE_H
