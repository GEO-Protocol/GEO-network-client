#ifndef GEO_NETWORK_CLIENT_RECORD_H
#define GEO_NETWORK_CLIENT_RECORD_H

#include "../../../../common/Types.h"

#include <boost/uuid/uuid.hpp>

namespace uuids = boost::uuids;

class Record {
public:
    typedef shared_ptr<Record> Shared;

public:
    enum RecordType {
        TrustLineRecordType = 1,
        PaymentRecordType
    };
    typedef uint8_t SerializedRecordType;

public:
    virtual const bool isTrustLineRecord() const;

    virtual const bool isPaymentRecord() const;

    const Record::RecordType recordType() const;

    const uuids::uuid operationUUID() const;

    virtual pair<BytesShared, size_t> serializeToBytes() = 0;

protected:
    Record();

    Record(
        const Record::RecordType recordType,
        const uuids::uuid &operationUUID);

    virtual size_t recordSize() = 0;

public:
    static const size_t kOperationUUIDBytesSize = 16;

private:
    RecordType mRecordType;
    uuids::uuid mOperationUUID;
};

#endif //GEO_NETWORK_CLIENT_RECORD_H
