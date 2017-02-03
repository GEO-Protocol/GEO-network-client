#ifndef GEO_NETWORK_CLIENT_OPENTRUSTLINEMESSAGE_H
#define GEO_NETWORK_CLIENT_OPENTRUSTLINEMESSAGE_H

#include "../../TrustLinesMessage.h"

#include "../../../../common/Types.h"

#include "../../../../common/NodeUUID.h"
#include "../../../../transactions/TransactionUUID.h"

#include <memory>
#include <utility>
#include <stdint.h>
#include <malloc.h>

class OpenTrustLineMessage : public TrustLinesMessage {

public:
    OpenTrustLineMessage(
        NodeUUID &sender,
        TransactionUUID &transactionUUID,
        TrustLineAmount amount);

    const MessageTypeID typeID() const;

    pair<ConstBytesShared, size_t> serialize();

private:
    void deserialize(
        byte* buffer);

private:
    TrustLineAmount mTrustLineAmount;
};


#endif //GEO_NETWORK_CLIENT_OPENTRUSTLINEMESSAGE_H
