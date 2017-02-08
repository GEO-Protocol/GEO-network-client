#ifndef GEO_NETWORK_CLIENT_SETTRUSTLINEMESSAGE_H
#define GEO_NETWORK_CLIENT_SETTRUSTLINEMESSAGE_H

#include "../../TrustLinesMessage.hpp"

#include "../../../../common/Types.h"
#include "../../../../common/memory/MemoryUtils.h"
#include "../../../../common/multiprecision/MultiprecisionUtils.h"

#include "../../../../common/NodeUUID.h"
#include "../../../../transactions/TransactionUUID.h"

#include <memory>
#include <utility>
#include <cstdlib>
#include <stdint.h>

class SetTrustLineMessage : public TrustLinesMessage {

public:
    SetTrustLineMessage(
        NodeUUID &sender,
        TransactionUUID &transactionUUID,
        TrustLineAmount newAmount);

    const MessageType typeID() const;

    pair<BytesShared, size_t> serializeToBytes();

private:
    void deserializeFromBytes(
        BytesShared buffer);

private:
    TrustLineAmount mNewTrustLineAmount;
};


#endif //GEO_NETWORK_CLIENT_SETTRUSTLINEMESSAGE_H
