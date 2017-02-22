#ifndef GEO_NETWORK_CLIENT_OPENTRUSTLINEMESSAGE_H
#define GEO_NETWORK_CLIENT_OPENTRUSTLINEMESSAGE_H

#include "../../TrustLinesMessage.hpp"

#include "../../../../common/Types.h"
#include "../../../../common/memory/MemoryUtils.h"
#include "../../../../common/multiprecision/MultiprecisionUtils.h"

#include "../../../../common/NodeUUID.h"
#include "../../../../transactions/transactions/base/TransactionUUID.h"

#include <memory>
#include <utility>
#include <cstdlib>
#include <stdint.h>

class OpenTrustLineMessage : public TrustLinesMessage {

public:
    OpenTrustLineMessage(
        NodeUUID &sender,
        TransactionUUID &transactionUUID,
        TrustLineAmount amount);

    const MessageType typeID() const;

    pair<BytesShared, size_t> serializeToBytes();

private:
    void deserializeFromBytes(
        BytesShared buffer);

private:
    TrustLineAmount mTrustLineAmount;
};


#endif //GEO_NETWORK_CLIENT_OPENTRUSTLINEMESSAGE_H
