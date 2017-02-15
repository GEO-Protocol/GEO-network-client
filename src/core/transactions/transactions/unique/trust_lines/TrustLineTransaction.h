#ifndef GEO_NETWORK_CLIENT_TRUSTLINETRANSACTION_H
#define GEO_NETWORK_CLIENT_TRUSTLINETRANSACTION_H

#include "../../base/UniqueTransaction.h"

#include "../../../../common/Types.h"
#include "../../../../common/memory/MemoryUtils.h"
#include "../../../../common/NodeUUID.h"

#include "../../../scheduler/TransactionsScheduler.h"

#include <cstdint>

class TrustLineTransaction : public UniqueTransaction {

protected:
    TrustLineTransaction(
        TransactionType type,
        NodeUUID &nodeUUID,
        TransactionsScheduler *scheduler);

    TrustLineTransaction(
        TransactionsScheduler *scheduler);

    void increaseRequestsCounter();

    void resetRequestsCounter();

    virtual pair<BytesShared, size_t> serializeToBytes() const;

protected:
    virtual void deserializeFromBytes(
        BytesShared buffer);

    static const size_t kOffsetToDataBytes();

protected:
    const uint16_t kResponsesCount = 1;
    const uint16_t kResponsePosition = 0;
    uint16_t mRequestCounter = 0;
};


#endif //GEO_NETWORK_CLIENT_TRUSTLINETRANSACTION_H
