#ifndef GEO_NETWORK_CLIENT_HISTORYSTORAGE_H
#define GEO_NETWORK_CLIENT_HISTORYSTORAGE_H

#include "../../common/NodeUUID.h"
#include "../../transactions/transactions/base/TransactionUUID.h"
#include "../../common/Types.h"
#include "../../logger/Logger.h"
#include "../../common/exceptions/IOError.h"
#include "../../common/exceptions/ValueError.h"
#include "../../common/multiprecision/MultiprecisionUtils.h"

#include "record/base/Record.h"
#include "record/payment/PaymentRecord.h"
#include "record/trust_line/TrustLineRecord.h"

#include "../../../libs/sqlite3/sqlite3.h"

#include <vector>
#include <memory>

class HistoryStorage {

public:
    HistoryStorage(
        sqlite3 *dbConnection,
        const string &mainTableName,
        const string &additionalTableName,
        Logger &logger);

    void saveTrustLineRecord(
        TrustLineRecord::Shared record);

    void savePaymentRecord(
        PaymentRecord::Shared record);

    vector<TrustLineRecord::Shared> allTrustLineRecords(
        size_t recordsCount,
        size_t fromRecord,
        DateTime timeFrom,
        bool isTimeFromPresent,
        DateTime timeTo,
        bool isTimeToPresent);

    vector<PaymentRecord::Shared> allPaymentRecords(
        size_t recordsCount,
        size_t fromRecord,
        DateTime timeFrom,
        bool isTimeFromPresent,
        DateTime timeTo,
        bool isTimeToPresent,
        const TrustLineAmount& lowBoundaryAmount,
        bool isLowBoundaryAmountPresent,
        const TrustLineAmount& highBoundaryAmount,
        bool isHighBoundaryAmountPresent);

    vector<PaymentRecord::Shared> paymentRecordsByCommandUUID(
        const CommandUUID &commandUUID);

    vector<PaymentRecord::Shared> allPaymentAdditionalRecords();

    vector<Record::Shared> recordsWithContractor(
        const NodeUUID &contractorUUID,
        size_t recordsCount,
        size_t fromRecord);

    const string mainTableName() const;
    const string additionalTableName() const;

private:
    void savePaymentMainOutgoingRecord(
        PaymentRecord::Shared record);

    void savePaymentMainIncomingRecord(
        PaymentRecord::Shared record);

    void savePaymentAdditionalRecord(
        PaymentRecord::Shared record);

    vector<PaymentRecord::Shared> allPaymentRecords(
        size_t recordsCount,
        size_t fromRecord,
        DateTime timeFrom,
        bool isTimeFromPresent,
        DateTime timeTo,
        bool isTimeToPresent);

    size_t countRecordsByType(
        Record::RecordType recordType);

    vector<Record::Shared> recordsPortionWithContractor(
        const NodeUUID &contractorUUID,
        size_t recordsCount,
        size_t fromRecord);

    pair<BytesShared, size_t> serializedTrustLineRecordBody(
        TrustLineRecord::Shared);

    pair<BytesShared, size_t> serializedPaymentRecordBody(
        PaymentRecord::Shared);

    pair<BytesShared, size_t> serializedPaymentAdditionalRecordBody(
        PaymentRecord::Shared);

    TrustLineRecord::Shared deserializeTrustLineRecord(
        sqlite3_stmt *stmt);

    PaymentRecord::Shared deserializePaymentRecord(
        sqlite3_stmt *stmt);

    PaymentRecord::Shared deserializePaymentAdditionalRecord(
        sqlite3_stmt *stmt);

    LoggerStream info() const;

    LoggerStream debug() const;

    LoggerStream error() const;

    const string logHeader() const;

private:
    const size_t kPortionRequestSize = 1000;

private:
    sqlite3 *mDataBase = nullptr;
    // main table used for storing history, needed for frontend
    // (trustlines, payments coordinator, payments receiver)
    string mMainTableName;
    // addidtional table used for storing history, needed for statistics
    // (cycles and payment intermediate nodes)
    string mAdditionalTableName;
    Logger &mLog;
};


#endif //GEO_NETWORK_CLIENT_HISTORYSTORAGE_H
