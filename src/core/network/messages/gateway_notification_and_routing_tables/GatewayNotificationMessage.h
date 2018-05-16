#ifndef GEO_NETWORK_CLIENT_GATEWAYNOTIFICATIONMESSAGE_H
#define GEO_NETWORK_CLIENT_GATEWAYNOTIFICATIONMESSAGE_H

#include "../base/transaction/TransactionMessage.h"

class GatewayNotificationMessage : public TransactionMessage {

public:
    typedef shared_ptr<GatewayNotificationMessage> Shared;

public:
    GatewayNotificationMessage(
        const NodeUUID &senderUUID,
        const TransactionUUID &transactionUUID,
        const vector<SerializedEquivalent> gatewayEquivalents);

    GatewayNotificationMessage(
        BytesShared buffer);

    const vector<SerializedEquivalent> gatewayEquivalents() const;

    const MessageType typeID() const;

    const bool isAddToConfirmationRequiredMessagesHandler() const;

protected:
    virtual pair<BytesShared, size_t> serializeToBytes() const;

protected:
    vector<SerializedEquivalent> mGatewayEquivalents;
};


#endif //GEO_NETWORK_CLIENT_GATEWAYNOTIFICATIONMESSAGE_H
