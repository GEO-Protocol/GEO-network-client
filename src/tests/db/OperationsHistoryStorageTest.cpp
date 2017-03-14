#include "OperationsHistoryStorageTest.h"

OperationsHistoryStorageTest::OperationsHistoryStorageTest() {

    mStorage = unique_ptr<st::OperationsHistoryStorage>(new st::OperationsHistoryStorage(
        "io",
        "operations_history.dat"));
}

st::Record::Shared OperationsHistoryStorageTest::createTrustLineRecord(
    st::TrustLineRecord::TrustLineOperationType operationType,
    TrustLineAmount amount) {

    uuids::uuid operationUUID;
    NodeUUID contractorUUID;

    return dynamic_pointer_cast<st::Record>(
        make_shared<st::TrustLineRecord>(
            operationUUID,
            operationType,
            contractorUUID,
            amount));
}

st::Record::Shared OperationsHistoryStorageTest::createPaymentRecord(st::PaymentRecord::PaymentOperationType operationType,
                                                                     TrustLineAmount amount) {

    uuids::uuid operationUUID;
    NodeUUID contractorUUID;

    return dynamic_pointer_cast<st::Record>(
        make_shared<st::PaymentRecord>(
            operationUUID,
            operationType,
            contractorUUID,
            amount));
}

void OperationsHistoryStorageTest::compareDataTestCase() {

    for (uint8_t number = 1; number <= 10; ++number) {

        st::Record::Shared record;

        if (number <= 3) {

            record = createTrustLineRecord(
                st::TrustLineRecord::TrustLineOperationType::Opening,
                TrustLineAmount(500 + number));

        }

        if (number > 3 && number <= 6) {

            record = createPaymentRecord(
                st::PaymentRecord::PaymentOperationType::IncomingPaymentType,
                TrustLineAmount(500 + number));

        }

        if (number > 6 && number <= 8) {

            record = createTrustLineRecord(
                st::TrustLineRecord::TrustLineOperationType::Closing,
                TrustLineAmount(500 + number));

        }

        if (number > 8 && number <= 10) {

            record = createPaymentRecord(
                st::PaymentRecord::PaymentOperationType::OutgoingPaymentType,
                TrustLineAmount(500 + number));

        }

        mRecords.push_back(record);

        mStorage->addRecord(
            record);

    }

    auto trustLineRecords = mStorage->recordsStack(
        st::Record::RecordType::TrustLineRecordType,
        5,
        0);

    auto paymentRecords = mStorage->recordsStack(
        st::Record::RecordType::PaymentRecordType,
        5,
        0);

    assert(trustLineRecords.size() == 5);
    assert(paymentRecords.size() == 5);

    assert(mRecords.at(0)->recordType() == trustLineRecords.at(0)->recordType());
    assert(mRecords.at(0)->operationUUID() == trustLineRecords.at(0)->operationUUID());

    assert(mRecords.at(1)->recordType() == trustLineRecords.at(1)->recordType());
    assert(mRecords.at(1)->operationUUID() == trustLineRecords.at(1)->operationUUID());

    assert(mRecords.at(2)->recordType() == trustLineRecords.at(2)->recordType());
    assert(mRecords.at(2)->operationUUID() == trustLineRecords.at(2)->operationUUID());

    assert(mRecords.at(3)->recordType() == paymentRecords.at(0)->recordType());
    assert(mRecords.at(3)->operationUUID() == paymentRecords.at(0)->operationUUID());

    assert(mRecords.at(4)->recordType() == paymentRecords.at(1)->recordType());
    assert(mRecords.at(4)->operationUUID() == paymentRecords.at(1)->operationUUID());

    assert(mRecords.at(5)->recordType() == paymentRecords.at(2)->recordType());
    assert(mRecords.at(5)->operationUUID() == paymentRecords.at(2)->operationUUID());

    assert(mRecords.at(6)->recordType() == trustLineRecords.at(3)->recordType());
    assert(mRecords.at(6)->operationUUID() == trustLineRecords.at(3)->operationUUID());

    assert(mRecords.at(7)->recordType() == trustLineRecords.at(4)->recordType());
    assert(mRecords.at(7)->operationUUID() == trustLineRecords.at(4)->operationUUID());

    assert(mRecords.at(8)->recordType() == paymentRecords.at(3)->recordType());
    assert(mRecords.at(8)->operationUUID() == paymentRecords.at(3)->operationUUID());

    assert(mRecords.at(9)->recordType() == paymentRecords.at(4)->recordType());
    assert(mRecords.at(9)->operationUUID() == paymentRecords.at(4)->operationUUID());

}

void OperationsHistoryStorageTest::run() {

    compareDataTestCase();
}