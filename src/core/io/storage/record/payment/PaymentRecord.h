#ifndef GEO_NETWORK_CLIENT_PAYMENTRECORD_H
#define GEO_NETWORK_CLIENT_PAYMENTRECORD_H

#include "../base/Record.h"

#include "../../../../interface/commands_interface/CommandUUID.h"
#include "../../../../common/memory/MemoryUtils.h"
#include "../../../../common/multiprecision/MultiprecisionUtils.h"

class PaymentRecord: public Record {
public:
    typedef shared_ptr<PaymentRecord> Shared;

public:
    enum PaymentOperationType {
        OutgoingPaymentType = 1,
        IncomingPaymentType,
    };
    typedef uint8_t SerializedPaymentOperationType;

public:
    PaymentRecord(
        const TransactionUUID &operationUUID,
        const PaymentRecord::PaymentOperationType operationType,
        Contractor::Shared contractor,
        const TrustLineAmount &amount,
        const TrustLineBalance &balanceAfterOperation);

    PaymentRecord(
        const TransactionUUID &operationUUID,
        const PaymentRecord::PaymentOperationType operationType,
        Contractor::Shared contractor,
        const TrustLineAmount &amount,
        const TrustLineBalance &balanceAfterOperation,
        const GEOEpochTimestamp geoEpochTimestamp);

    PaymentRecord(
        const TransactionUUID &operationUUID,
        const PaymentRecord::PaymentOperationType operationType,
        Contractor::Shared contractor,
        const TrustLineAmount &amount,
        const TrustLineBalance &balanceAfterOperation,
        const CommandUUID &commandUUID);

    PaymentRecord(
        const TransactionUUID &operationUUID,
        const PaymentRecord::PaymentOperationType operationType,
        Contractor::Shared contractor,
        const TrustLineAmount &amount,
        const TrustLineBalance &balanceAfterOperation,
        const CommandUUID &commandUUID,
        const GEOEpochTimestamp geoEpochTimestamp);

    const PaymentOperationType paymentOperationType() const;

    const TrustLineAmount &amount() const;

    const TrustLineBalance &balanceAfterOperation() const;

    const CommandUUID &commandUUID() const;

    const bool isPaymentRecord() const;

    pair<BytesShared, size_t> serializedHistoryRecordBody() const override;

private:
    PaymentOperationType mPaymentOperationType;
    TrustLineAmount mAmount;
    TrustLineBalance mBalanceAfterOperation;
    CommandUUID mCommandUUID;
};

#endif //GEO_NETWORK_CLIENT_PAYMENTRECORD_H
