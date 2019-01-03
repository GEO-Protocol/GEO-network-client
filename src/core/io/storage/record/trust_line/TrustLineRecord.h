#ifndef GEO_NETWORK_CLIENT_TRUSTLINERECORD_H
#define GEO_NETWORK_CLIENT_TRUSTLINERECORD_H

#include "../base/Record.h"

#include "../../../../common/memory/MemoryUtils.h"
#include "../../../../common/multiprecision/MultiprecisionUtils.h"

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
        Contractor::Shared contractor);

    TrustLineRecord(
        const TransactionUUID &operationUUID,
        const TrustLineRecord::TrustLineOperationType operationType,
        Contractor::Shared contractor,
        const TrustLineAmount &amount);

    TrustLineRecord(
        const TransactionUUID &operationUUID,
        const TrustLineRecord::TrustLineOperationType operationType,
        Contractor::Shared contractor,
        const TrustLineAmount &amount,
        const GEOEpochTimestamp geoEpochTimestamp);

    const TrustLineOperationType trustLineOperationType() const;

    const TrustLineAmount amount() const;

    const bool isTrustLineRecord() const;

    pair<BytesShared, size_t> serializedHistoryRecordBody() const override;

private:
    TrustLineOperationType mTrustLineOperationType;
    TrustLineAmount mAmount;
};

#endif //GEO_NETWORK_CLIENT_TRUSTLINERECORD_H
