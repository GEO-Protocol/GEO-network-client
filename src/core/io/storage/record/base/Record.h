#ifndef GEO_NETWORK_CLIENT_RECORD_H
#define GEO_NETWORK_CLIENT_RECORD_H

#include "../../../../common/Types.h"
#include "../../../../transactions/transactions/base/TransactionUUID.h"
#include "../../../../common/time/TimeUtils.h"

class Record {
public:
    typedef shared_ptr<Record> Shared;

public:
    enum RecordType {
        TrustLineRecordType = 1,
        PaymentRecordType
    };

public:
    virtual const bool isTrustLineRecord() const;

    virtual const bool isPaymentRecord() const;

    const Record::RecordType recordType() const;

    const TransactionUUID operationUUID() const;

    const DateTime timestamp() const;

    const NodeUUID contractorUUID() const;

protected:
    Record();

    Record(
        const Record::RecordType recordType,
        const TransactionUUID &operationUUID,
        const NodeUUID &contractorUUID);

    Record(
        const Record::RecordType recordType,
        const TransactionUUID &operationUUID,
        const NodeUUID &contractorUUID,
        const GEOEpochTimestamp geoEpochTimestamp);

public:
    static const size_t kOperationUUIDBytesSize = 16;

private:
    RecordType mRecordType;
    TransactionUUID mOperationUUID;
    DateTime mTimestamp;
    NodeUUID mContractorUUID;
};

#endif //GEO_NETWORK_CLIENT_RECORD_H
