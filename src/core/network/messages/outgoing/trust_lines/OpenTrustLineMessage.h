#ifndef GEO_NETWORK_CLIENT_OPENTRUSTLINEMESSAGE_H
#define GEO_NETWORK_CLIENT_OPENTRUSTLINEMESSAGE_H

#include "../../base/trust_lines/TrustLinesMessage.h"

#include "../../../../common/Types.h"
#include "../../../../common/NodeUUID.h"
#include "../../../../common/memory/MemoryUtils.h"
#include "../../../../common/multiprecision/MultiprecisionUtils.h"

#include "../../../../transactions/transactions/base/TransactionUUID.h"

#include <memory>
#include <utility>
#include <stdint.h>

class OpenTrustLineMessage : public TrustLinesMessage {

public:
    OpenTrustLineMessage(
        const NodeUUID &sender,
        const TransactionUUID &transactionUUID,
        const TrustLineAmount &amount);

private:
    const MessageType typeID() const;

    pair<BytesShared, size_t> serializeToBytes();

    void deserializeFromBytes(
        BytesShared buffer);

private:
    TrustLineAmount mTrustLineAmount;
};


#endif //GEO_NETWORK_CLIENT_OPENTRUSTLINEMESSAGE_H
