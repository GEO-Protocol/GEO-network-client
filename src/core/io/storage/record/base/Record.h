#ifndef GEO_NETWORK_CLIENT_RECORD_H
#define GEO_NETWORK_CLIENT_RECORD_H

#include "../../../../common/Types.h"
#include "../../../../transactions/transactions/base/TransactionUUID.h"
#include "../../../../contractors/Contractor.h"
#include "../../../../common/time/TimeUtils.h"

class Record {
public:
    typedef shared_ptr<Record> Shared;

public:
    enum RecordType {
        TrustLineRecordType = 1,
        PaymentRecordType,
        PaymentAdditionalRecordType,
    };

public:
    virtual const bool isTrustLineRecord() const;

    virtual const bool isPaymentRecord() const;

    const Record::RecordType recordType() const;

    const TransactionUUID operationUUID() const;

    const DateTime timestamp() const;

    Contractor::Shared contractor() const;

    virtual pair<BytesShared, size_t> serializedHistoryRecordBody() const = 0;

protected:
    Record(
        const Record::RecordType recordType,
        const TransactionUUID &operationUUID,
        Contractor::Shared contractor);

    Record(
        const Record::RecordType recordType,
        const TransactionUUID &operationUUID,
        Contractor::Shared contractor,
        const GEOEpochTimestamp geoEpochTimestamp);

    Record(
        const Record::RecordType recordType,
        const TransactionUUID &operationUUID,
        const GEOEpochTimestamp geoEpochTimestamp);

protected:
    RecordType mRecordType;
    TransactionUUID mOperationUUID;
    DateTime mTimestamp;
    Contractor::Shared mContractor;
};

#endif //GEO_NETWORK_CLIENT_RECORD_H
