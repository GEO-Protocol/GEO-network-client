/**
 * This file is part of GEO Protocol.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Protocol/GEO-network-client/blob/master/LICENSE.md
 *
 * No part of GEO Protocol, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#include "PaymentRecord.h"

PaymentRecord::PaymentRecord(
    const TransactionUUID &operationUUID,
    const PaymentRecord::PaymentOperationType operationType,
    const NodeUUID &contractorUUID,
    const TrustLineAmount &amount,
    const TrustLineBalance &balanceAfterOperation):

    Record(
        Record::RecordType::PaymentRecordType,
        operationUUID,
        contractorUUID),
    mPaymentOperationType(operationType),
    mAmount(amount),
    mBalanceAfterOperation(balanceAfterOperation),
    mCommandUUID(CommandUUID::empty())
{}

PaymentRecord::PaymentRecord(
    const TransactionUUID &operationUUID,
    const PaymentRecord::PaymentOperationType operationType,
    const NodeUUID &contractorUUID,
    const TrustLineAmount &amount,
    const TrustLineBalance &balanceAfterOperation,
    const GEOEpochTimestamp geoEpochTimestamp):

    Record(
        Record::RecordType::PaymentRecordType,
        operationUUID,
        contractorUUID,
        geoEpochTimestamp),
    mPaymentOperationType(operationType),
    mAmount(amount),
    mBalanceAfterOperation(balanceAfterOperation),
    mCommandUUID(CommandUUID::empty())
{}

PaymentRecord::PaymentRecord(
    const TransactionUUID &operationUUID,
    const PaymentRecord::PaymentOperationType operationType,
    const TrustLineAmount &amount):

    Record(
        Record::RecordType::PaymentRecordType,
        operationUUID,
        NodeUUID::empty()),
    mPaymentOperationType(operationType),
    mAmount(amount),
    mBalanceAfterOperation(0),
    mCommandUUID(CommandUUID::empty())
{}

PaymentRecord::PaymentRecord(
    const TransactionUUID &operationUUID,
    const PaymentRecord::PaymentOperationType operationType,
    const TrustLineAmount &amount,
    const GEOEpochTimestamp geoEpochTimestamp):

    Record(
        Record::RecordType::PaymentRecordType,
        operationUUID,
        NodeUUID::empty(),
        geoEpochTimestamp),
    mPaymentOperationType(operationType),
    mAmount(amount),
    mBalanceAfterOperation(0),
    mCommandUUID(CommandUUID::empty())
{}

PaymentRecord::PaymentRecord(
    const TransactionUUID &operationUUID,
    const PaymentRecord::PaymentOperationType operationType,
    const NodeUUID &contractorUUID,
    const TrustLineAmount &amount,
    const TrustLineBalance &balanceAfterOperation,
    const CommandUUID &commandUUID):

    Record(
        Record::RecordType::PaymentRecordType,
        operationUUID,
        contractorUUID),
    mPaymentOperationType(operationType),
    mAmount(amount),
    mBalanceAfterOperation(balanceAfterOperation),
    mCommandUUID(commandUUID)
{}

PaymentRecord::PaymentRecord(
    const TransactionUUID &operationUUID,
    const PaymentRecord::PaymentOperationType operationType,
    const NodeUUID &contractorUUID,
    const TrustLineAmount &amount,
    const TrustLineBalance &balanceAfterOperation,
    const CommandUUID &commandUUID,
    const GEOEpochTimestamp geoEpochTimestamp):

    Record(
        Record::RecordType::PaymentRecordType,
        operationUUID,
        contractorUUID,
        geoEpochTimestamp),
    mPaymentOperationType(operationType),
    mAmount(amount),
    mBalanceAfterOperation(balanceAfterOperation),
    mCommandUUID(commandUUID)
{}

const bool PaymentRecord::isPaymentRecord() const
{
    return true;
}

const PaymentRecord::PaymentOperationType PaymentRecord::paymentOperationType() const
{
    return mPaymentOperationType;
}

const TrustLineAmount& PaymentRecord::amount() const
{
    return mAmount;
}

const TrustLineBalance& PaymentRecord::balanceAfterOperation() const
{
    return mBalanceAfterOperation;
}

const CommandUUID& PaymentRecord::commandUUID() const
{
    return mCommandUUID;
}
