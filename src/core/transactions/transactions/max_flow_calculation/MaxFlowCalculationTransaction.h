//
// Created by mc on 14.02.17.
//

#ifndef GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONTRANSACTION_H
#define GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONTRANSACTION_H


#include "../base/BaseTransaction.h"
#include "../../../trust_lines/manager/TrustLinesManager.h"

class MaxFlowCalculationTransaction : public BaseTransaction {

protected:
    MaxFlowCalculationTransaction(
            TransactionType type,
            NodeUUID &nodeUUID);

    MaxFlowCalculationTransaction();

    virtual pair<BytesShared, size_t> serializeToBytes() const;

    virtual void deserializeFromBytes(
        BytesShared buffer);

public:
    static const size_t kOffsetToDataBytes();

};


#endif //GEO_NETWORK_CLIENT_MAXFLOWCALCULATIONTRANSACTION_H
