/**
 * This file is part of GEO Protocol.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Protocol/GEO-network-client/blob/master/LICENSE.md
 *
 * No part of GEO Protocol, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#ifndef GEO_NETWORK_CLIENT_TRUSTLINERECORD_H
#define GEO_NETWORK_CLIENT_TRUSTLINERECORD_H

#include "../base/Record.h"

#include "../../../../common/Types.h"
#include "../../../../common/NodeUUID.h"
#include "../../../../transactions/transactions/base/TransactionUUID.h"
#include "../../../../common/memory/MemoryUtils.h"
#include "../../../../common/multiprecision/MultiprecisionUtils.h"

#include <memory>
#include <vector>

class TrustLineRecord: public Record {
public:
    typedef shared_ptr<TrustLineRecord> Shared;

public:
    enum TrustLineOperationType {
        Opening = 1,
        Accepting,
        Setting,
        Updating,
        Closing,
        Rejecting,
        ClosingIncoming,
        RejectingOutgoing
    };
    typedef uint8_t SerializedTrustLineOperationType;

public:
    TrustLineRecord(
        const TransactionUUID &operationUUID,
        const TrustLineRecord::TrustLineOperationType operationType,
        const NodeUUID &contractorUUID);

    TrustLineRecord(
        const TransactionUUID &operationUUID,
        const TrustLineRecord::TrustLineOperationType operationType,
        const NodeUUID &contractorUUID,
        const TrustLineAmount &amount);

    TrustLineRecord(
        const TransactionUUID &operationUUID,
        const TrustLineRecord::TrustLineOperationType operationType,
        const NodeUUID &contractorUUID,
        const TrustLineAmount &amount,
        const GEOEpochTimestamp geoEpochTimestamp);

    const TrustLineOperationType trustLineOperationType() const;

    const TrustLineAmount amount() const;

    const bool isTrustLineRecord() const;

private:
    TrustLineOperationType mTrustLineOperationType;
    TrustLineAmount mAmount;
};

#endif //GEO_NETWORK_CLIENT_TRUSTLINERECORD_H
