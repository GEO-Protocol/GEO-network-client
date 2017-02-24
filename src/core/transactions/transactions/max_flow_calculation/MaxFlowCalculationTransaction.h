#ifndef GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONTRANSACTION_H
#define GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONTRANSACTION_H

#include "../base/BaseTransaction.h"

#include "../../../common/Types.h"
#include "../../../common/NodeUUID.h"

#include "../../../trust_lines/manager/TrustLinesManager.h"

#include <utility>
#include <stdint.h>

class MaxFlowCalculationTransaction : public BaseTransaction {

protected:
    MaxFlowCalculationTransaction(
            const TransactionType type,
            const NodeUUID &nodeUUID);

    MaxFlowCalculationTransaction(
        const TransactionType type);

    virtual pair<BytesShared, size_t> serializeToBytes() const;

    virtual void deserializeFromBytes(
        BytesShared buffer);

public:
    static const size_t kOffsetToDataBytes();

};


#endif //GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONTRANSACTION_H
