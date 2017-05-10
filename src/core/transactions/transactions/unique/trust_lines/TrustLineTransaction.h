#ifndef GEO_NETWORK_CLIENT_TRUSTLINETRANSACTION_H
#define GEO_NETWORK_CLIENT_TRUSTLINETRANSACTION_H

#include "../../base/BaseTransaction.h"

#include "../../../../common/Types.h"
#include "../../../../common/NodeUUID.h"
#include "../../../../common/memory/MemoryUtils.h"

#include <utility>
#include <stdint.h>

class TrustLineTransaction : public BaseTransaction {

protected:
    TrustLineTransaction(
        const BaseTransaction::TransactionType type,
        const NodeUUID &nodeUUID,
        Logger *logger);

    TrustLineTransaction(
        const BaseTransaction::TransactionType type,
        Logger *logger);

    void increaseRequestsCounter();

    void resetRequestsCounter();

    virtual pair<BytesShared, size_t> serializeToBytes() const;

protected:
    virtual void deserializeFromBytes(
        BytesShared buffer);

    const size_t kOffsetToDataBytes();

protected:
    const uint16_t kResponsesCount = 1;
    const uint16_t kResponsePosition = 0;

    uint16_t mRequestCounter = 0;
};


#endif //GEO_NETWORK_CLIENT_TRUSTLINETRANSACTION_H
