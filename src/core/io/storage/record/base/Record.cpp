#include "Record.h"

Record::Record() {}

Record::Record(
    const Record::RecordType recordType,
    const uuids::uuid &operationUUID):

    mOperationUUID(operationUUID)
{

    mRecordType = recordType;
}

const bool Record::isTrustLineRecord() const {

    return false;
}

const bool Record::isPaymentRecord() const {
            return false;
}

const Record::RecordType Record::recordType() const {

    return mRecordType;
}

const uuids::uuid Record::operationUUID() const {

    return mOperationUUID;
}
