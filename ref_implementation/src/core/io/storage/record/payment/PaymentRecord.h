/**
 * This file is part of GEO Protocol.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Protocol/GEO-network-client/blob/master/LICENSE.md
 *
 * No part of GEO Protocol, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#ifndef GEO_NETWORK_CLIENT_PAYMENTRECORD_H
#define GEO_NETWORK_CLIENT_PAYMENTRECORD_H

#include "../base/Record.h"

#include "../../../../common/Types.h"
#include "../../../../common/NodeUUID.h"
#include "../../../../interface/commands_interface/CommandUUID.h"
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
        IncomingPaymentType,
        IntermediatePaymentType,
        CycleCloserType,
        CyclerCloserIntermediateType
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

    PaymentRecord(
        const TransactionUUID &operationUUID,
        const PaymentRecord::PaymentOperationType operationType,
        const TrustLineAmount &amount);

    PaymentRecord(
        const TransactionUUID &operationUUID,
        const PaymentRecord::PaymentOperationType operationType,
        const TrustLineAmount &amount,
        const GEOEpochTimestamp geoEpochTimestamp);

    PaymentRecord(
        const TransactionUUID &operationUUID,
        const PaymentRecord::PaymentOperationType operationType,
        const NodeUUID &contractorUUID,
        const TrustLineAmount &amount,
        const TrustLineBalance &balanceAfterOperation,
        const CommandUUID &commandUUID);

    PaymentRecord(
        const TransactionUUID &operationUUID,
        const PaymentRecord::PaymentOperationType operationType,
        const NodeUUID &contractorUUID,
        const TrustLineAmount &amount,
        const TrustLineBalance &balanceAfterOperation,
        const CommandUUID &commandUUID,
        const GEOEpochTimestamp geoEpochTimestamp);

    const PaymentOperationType paymentOperationType() const;

    const TrustLineAmount &amount() const;

    const TrustLineBalance &balanceAfterOperation() const;

    const CommandUUID &commandUUID() const;

    const bool isPaymentRecord() const;

private:
    PaymentOperationType mPaymentOperationType;
    TrustLineAmount mAmount;
    TrustLineBalance mBalanceAfterOperation;
    CommandUUID mCommandUUID;
};

#endif //GEO_NETWORK_CLIENT_PAYMENTRECORD_H
