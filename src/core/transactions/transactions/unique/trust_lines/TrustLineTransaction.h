#ifndef GEO_NETWORK_CLIENT_TRUSTLINETRANSACTION_H
#define GEO_NETWORK_CLIENT_TRUSTLINETRANSACTION_H

#include "../../base/BaseTransaction.h"

#include "../../../../common/Types.h"
#include "../../../../common/NodeUUID.h"
#include "../../../../common/memory/MemoryUtils.h"

#include <utility>
#include <stdint.h>


class TrustLineTransaction:
    public BaseTransaction {

protected:
    TrustLineTransaction(
        const BaseTransaction::TransactionType type,
        const NodeUUID &nodeUUID,
        Logger &logger);

    TrustLineTransaction(
        const BaseTransaction::TransactionType type,
        Logger &logger);

protected:
    const uint16_t kResponsesCount = 1;
};


#endif //GEO_NETWORK_CLIENT_TRUSTLINETRANSACTION_H
