#ifndef GEO_NETWORK_CLIENT_OPENTRUSTLINEMESSAGE_H
#define GEO_NETWORK_CLIENT_OPENTRUSTLINEMESSAGE_H

#include "Message.h"

#include "../../transactions/TransactionUUID.h"
#include "../../trust_lines/TrustLine.h"

class OpenTrustLineMessage : public Message {

public:
    OpenTrustLineMessage(
        TransactionUUID &transactionUUID,
        TrustLineAmount &amount);

    pair<ConstBytesShared, size_t> serialize() const;

    const MessageTypeID typeID() const;

private:
    const size_t kTrustLineAmountSize = 32;

    TransactionUUID mTransactionUUID;
    TrustLineAmount mTrustLineAmount;
};


#endif //GEO_NETWORK_CLIENT_OPENTRUSTLINEMESSAGE_H
