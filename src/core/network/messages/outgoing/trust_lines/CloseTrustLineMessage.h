#ifndef GEO_NETWORK_CLIENT_CLOSETRUSTLINEMESSAGE_H
#define GEO_NETWORK_CLIENT_CLOSETRUSTLINEMESSAGE_H

#include "../../base/trust_lines/TrustLinesMessage.h"

#include "../../../../common/Types.h"
#include "../../../../common/NodeUUID.h"
#include "../../../../common/memory/MemoryUtils.h"
#include "../../../../common/multiprecision/MultiprecisionUtils.h"

#include "../../../../transactions/transactions/base/TransactionUUID.h"

#include <memory>
#include <utility>
#include <stdint.h>

class CloseTrustLineMessage: public TrustLinesMessage {

public:
    CloseTrustLineMessage(
        const NodeUUID &sender,
        const TransactionUUID &transactionUUID,
        const NodeUUID &contractorUUID
    );

private:
    const MessageType typeID() const;

    pair<BytesShared, size_t> serializeToBytes();

    void deserializeFromBytes(
        BytesShared buffer);

private:
    NodeUUID mContractorUUID;
};


#endif //GEO_NETWORK_CLIENT_CLOSETRUSTLINEMESSAGE_H
