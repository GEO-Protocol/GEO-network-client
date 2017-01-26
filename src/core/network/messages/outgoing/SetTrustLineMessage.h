#ifndef GEO_NETWORK_CLIENT_SETTRUSTLINEMESSAGE_H
#define GEO_NETWORK_CLIENT_SETTRUSTLINEMESSAGE_H

#include "../Message.h"

#include "../../../common/Types.h"

#include "../../../transactions/TransactionUUID.h"
#include "../../../common/NodeUUID.h"

class SetTrustLineMessage : public Message {

public:
    SetTrustLineMessage(
        NodeUUID &sender,
        TransactionUUID &transactionUUID,
        TrustLineAmount newAmount);

    pair<ConstBytesShared, size_t> serialize();

    void deserialize(
        byte* buffer);

    const MessageTypeID typeID() const;

private:
    const size_t kTrustLineAmountSize = 32;

    TrustLineAmount mNewTrustLineAmount;
};


#endif //GEO_NETWORK_CLIENT_SETTRUSTLINEMESSAGE_H
