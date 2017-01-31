#ifndef GEO_NETWORK_CLIENT_OPENTRUSTLINEMESSAGE_H
#define GEO_NETWORK_CLIENT_OPENTRUSTLINEMESSAGE_H

#include "../../Message.h"

#include "../../../../common/Types.h"

#include "../../../../transactions/TransactionUUID.h"
#include "../../../../common/NodeUUID.h"

class OpenTrustLineMessage : public Message {

public:
    OpenTrustLineMessage(
        NodeUUID &sender,
        TransactionUUID &transactionUUID,
        TrustLineAmount amount);

    pair<ConstBytesShared, size_t> serialize();

    void deserialize(
        byte* buffer);

    const MessageTypeID typeID() const;

private:
    const size_t kTrustLineAmountSize = 32;

    TrustLineAmount mTrustLineAmount;
};


#endif //GEO_NETWORK_CLIENT_OPENTRUSTLINEMESSAGE_H
