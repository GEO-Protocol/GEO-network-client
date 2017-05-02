#ifndef GEO_NETWORK_CLIENT_PAYMENTRECORD_H
#define GEO_NETWORK_CLIENT_PAYMENTRECORD_H

#include "../base/Record.h"

#include "../../../../common/Types.h"
#include "../../../../common/NodeUUID.h"
#include "../../../../transactions/transactions/base/TransactionUUID.h"
#include "../../../../common/memory/MemoryUtils.h"
#include "../../../../common/multiprecision/MultiprecisionUtils.h"

#include <memory>
#include <vector>

class PaymentRecord: public Record {
public:
    typedef shared_ptr<PaymentRecord> Shared;

public:
    enum PaymentOperationType {
        OutgoingPaymentType = 1,
        IncomingPaymentType
    };
    typedef uint8_t SerializedPaymentOperationType;

public:
    PaymentRecord(
        const TransactionUUID &operationUUID,
        const PaymentRecord::PaymentOperationType operationType,
        const NodeUUID &contractorUUID,
        const TrustLineAmount &amount,
        const TrustLineBalance &balanceAfterOperation);

    PaymentRecord(
        const TransactionUUID &operationUUID,
        const PaymentRecord::PaymentOperationType operationType,
        const NodeUUID &contractorUUID,
        const TrustLineAmount &amount,
        const TrustLineBalance &balanceAfterOperation,
        const GEOEpochTimestamp geoEpochTimestamp);

    const PaymentOperationType paymentOperationType() const;

    const NodeUUID contractorUUID() const;

    const TrustLineAmount amount() const;

    const TrustLineBalance balanceAfterOperation() const;

    const bool isPaymentRecord() const;

private:
    PaymentOperationType mPaymentOperationType;
    NodeUUID mContractorUUID;
    TrustLineAmount mAmount;
    TrustLineBalance mBalanceAfterOperation;
};

#endif //GEO_NETWORK_CLIENT_PAYMENTRECORD_H
