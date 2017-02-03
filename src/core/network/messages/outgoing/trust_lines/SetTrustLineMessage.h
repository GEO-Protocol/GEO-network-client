#ifndef GEO_NETWORK_CLIENT_SETTRUSTLINEMESSAGE_H
#define GEO_NETWORK_CLIENT_SETTRUSTLINEMESSAGE_H

#include "../../TrustLinesMessage.h"

#include "../../../../common/Types.h"

#include "../../../../common/NodeUUID.h"
#include "../../../../transactions/TransactionUUID.h"

#include <memory>
#include <utility>
#include <stdint.h>
#include <malloc.h>

class SetTrustLineMessage : public TrustLinesMessage {

public:
    SetTrustLineMessage(
        NodeUUID &sender,
        TransactionUUID &transactionUUID,
        TrustLineAmount newAmount);

    const MessageTypeID typeID() const;

    pair<ConstBytesShared, size_t> serialize();

private:
    void deserialize(
        byte* buffer);

private:
    TrustLineAmount mNewTrustLineAmount;
};


#endif //GEO_NETWORK_CLIENT_SETTRUSTLINEMESSAGE_H
