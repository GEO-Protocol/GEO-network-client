#ifndef GEO_NETWORK_CLIENT_TRUSTLINERECORD_H
#define GEO_NETWORK_CLIENT_TRUSTLINERECORD_H

#include "../base/Record.h"

#include "../../../../common/Types.h"
#include "../../../../common/NodeUUID.h"
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
        Rejecting
    };
    typedef uint8_t SerializedTrustLineOperationType;

public:
    TrustLineRecord(
        const uuids::uuid &operationUUID,
        BytesShared buffer);

    TrustLineRecord(
        const uuids::uuid &operationUUID,
        const TrustLineRecord::TrustLineOperationType operationType,
        const NodeUUID &contractorUUID);

    TrustLineRecord(
        const uuids::uuid &operationUUID,
        const TrustLineRecord::TrustLineOperationType operationType,
        const NodeUUID &contractorUUID,
        const TrustLineAmount &amount);

    const TrustLineOperationType trustLineOperationType() const;

    const NodeUUID contractorUUID() const;

    const TrustLineAmount amount() const;

    pair<BytesShared, size_t> serializeToBytes();

    const bool isTrustLineRecord() const;

private:

    size_t recordSize();

private:
    TrustLineOperationType mTrustLineOperationType;
    NodeUUID mContractorUUID;
    TrustLineAmount mAmount;
};

#endif //GEO_NETWORK_CLIENT_TRUSTLINERECORD_H
