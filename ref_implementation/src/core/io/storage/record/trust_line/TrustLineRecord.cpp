/**
 * This file is part of GEO Protocol.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Protocol/GEO-network-client/blob/master/LICENSE.md
 *
 * No part of GEO Protocol, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#include "TrustLineRecord.h"

TrustLineRecord::TrustLineRecord(
    const TransactionUUID &operationUUID,
    const TrustLineRecord::TrustLineOperationType operationType,
    const NodeUUID &contractorUUID):

    Record(
        Record::RecordType::TrustLineRecordType,
        operationUUID,
        contractorUUID),
    mTrustLineOperationType(operationType),
    mAmount(0)
{}

TrustLineRecord::TrustLineRecord(
    const TransactionUUID &operationUUID,
    const TrustLineRecord::TrustLineOperationType operationType,
    const NodeUUID &contractorUUID,
    const TrustLineAmount &amount):

    Record(
        Record::RecordType::TrustLineRecordType,
        operationUUID,
        contractorUUID),
    mTrustLineOperationType(operationType),
    mAmount(amount)
{}

TrustLineRecord::TrustLineRecord(
    const TransactionUUID &operationUUID,
    const TrustLineRecord::TrustLineOperationType operationType,
    const NodeUUID &contractorUUID,
    const TrustLineAmount &amount,
    const GEOEpochTimestamp geoEpochTimestamp) :

    Record(
        Record::RecordType::TrustLineRecordType,
        operationUUID,
        contractorUUID,
        geoEpochTimestamp),
    mTrustLineOperationType(operationType),
    mAmount(amount)
{}

const bool TrustLineRecord::isTrustLineRecord() const
{
    return true;
}

const TrustLineRecord::TrustLineOperationType TrustLineRecord::trustLineOperationType() const
{
    return mTrustLineOperationType;
}

const TrustLineAmount TrustLineRecord::amount() const
{
    return mAmount;
}
