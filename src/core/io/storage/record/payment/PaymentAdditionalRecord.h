#ifndef GEO_NETWORK_CLIENT_PAYMENTADDITIONALRECORD_H
#define GEO_NETWORK_CLIENT_PAYMENTADDITIONALRECORD_H

#include "../base/Record.h"

#include "../../../../common/memory/MemoryUtils.h"
#include "../../../../common/multiprecision/MultiprecisionUtils.h"

class PaymentAdditionalRecord : public Record {

public:
    typedef shared_ptr<PaymentAdditionalRecord> Shared;

public:
    enum PaymentAdditionalOperationType {
        IntermediatePaymentType = 1,
        CycleCloserType,
        CycleCloserIntermediateType,
    };
    typedef uint8_t SerializedPaymentOperationType;

    PaymentAdditionalRecord(
        const TransactionUUID &operationUUID,
        const PaymentAdditionalOperationType operationType,
        const TrustLineAmount &amount);

    PaymentAdditionalRecord(
        const TransactionUUID &operationUUID,
        const PaymentAdditionalOperationType operationType,
        const TrustLineAmount &amount,
        const GEOEpochTimestamp geoEpochTimestamp);

    const PaymentAdditionalOperationType operationType() const;

    const TrustLineAmount &amount() const;

    pair<BytesShared, size_t> serializedHistoryRecordBody() const override;

private:
    PaymentAdditionalOperationType mPaymentOperationType;
    TrustLineAmount mAmount;
};


#endif //GEO_NETWORK_CLIENT_PAYMENTADDITIONALRECORD_H
