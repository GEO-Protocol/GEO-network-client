#include "OperationsHistoryStorageTest.h"

OperationsHistoryStorageTest::OperationsHistoryStorageTest() {

    mStorage = unique_ptr<st::OperationsHistoryStorage>(new st::OperationsHistoryStorage(
        "io",
        "operations_history.dat"));
}

void OperationsHistoryStorageTest::addFewRecords(
    uint8_t count) {

    for (uint8_t number = 0; number < count; ++number) {

        uuids::uuid operationUUID;
        NodeUUID contractorUUID;

        st::Record::Shared record;

        if (number % 2 > 0) {
            record = make_shared<st::TrustLineRecord>(
                operationUUID,
                st::TrustLineRecord::TrustLineOperationType::Opening,
                contractorUUID,
                TrustLineAmount(500));

        } else {
            record = make_shared<st::PaymentRecord>(
                operationUUID,
                st::PaymentRecord::PaymentOperationType::IncomingPaymentType,
                contractorUUID,
                TrustLineAmount(500));
        }

        mOperationsUUIDs.push_back(
            operationUUID);
        mContractorsUUIDs.push_back(
            contractorUUID);

        mStorage->addRecord(
            record);

    }
}

vector<st::Record::Shared> OperationsHistoryStorageTest::fetchRecords(st::Record::RecordType recordType,
                                                                           size_t recordsCount,
                                                                           size_t fromRecord) {

    /*return mStorage->recordsStack(
        recordType,
        recordsCount,
        fromRecord);*/
}

void OperationsHistoryStorageTest::compareDataTestCase() {

    const uint8_t trustLineRecordsCount = 3;
    const uint8_t paymentRecordsCount = 2;
    uint8_t totalRecordsCount = trustLineRecordsCount + paymentRecordsCount;

    addFewRecords(
        totalRecordsCount);

    auto trustRecords = fetchRecords(
        st::Record::RecordType::TrustLineRecordType,
        3,
        0);

    auto paymentRecords = fetchRecords(
        st::Record::RecordType::PaymentRecordType,
        2,
        0);

    assert(trustRecords.size() == trustLineRecordsCount);
    assert(paymentRecords.size() == paymentRecordsCount);

    for (const auto record : trustRecords) {

        if (record->recordType() != st::Record::RecordType::TrustLineRecordType) {
            assert(false);
        }

        auto trustLineRecord = dynamic_pointer_cast<st::TrustLineRecord>(record);

        bool wasMatching = false;
        for (const auto& operationUUID : mOperationsUUIDs) {

            if (operationUUID == trustLineRecord->operationUUID()) {
                wasMatching = true;
                break;
            }

        }
        assert(wasMatching);

        wasMatching = false;
        for (const auto& contractorUUID : mContractorsUUIDs) {

            if (contractorUUID == trustLineRecord->contractorUUID()) {
                wasMatching = true;
                break;
            }

        }
        assert(wasMatching);

        assert(trustLineRecord->trustLineOperationType() == st::PaymentRecord::PaymentOperationType::IncomingPaymentType);
        assert(trustLineRecord->amount() == TrustLineAmount(500));

    }

}

void OperationsHistoryStorageTest::run() {

    compareDataTestCase();
}
