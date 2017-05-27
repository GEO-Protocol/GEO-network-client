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
        const string &tableName,
        Logger *logger);

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

private:
    vector<PaymentRecord::Shared> allPaymentRecords(
        size_t recordsCount,
        size_t fromRecord,
        DateTime timeFrom,
        bool isTimeFromPresent,
        DateTime timeTo,
        bool isTimeToPresent);

    size_t countRecordsByType(
        Record::RecordType recordType);

    pair<BytesShared, size_t> serializedTrustLineRecordBody(
        TrustLineRecord::Shared);

    pair<BytesShared, size_t> serializedPaymentRecordBody(
        PaymentRecord::Shared);

    TrustLineRecord::Shared deserializeTrustLineRecord(
        sqlite3_stmt *stmt);

    PaymentRecord::Shared deserializePaymentRecord(
        sqlite3_stmt *stmt);

    LoggerStream info() const;

    LoggerStream error() const;

    const string logHeader() const;

private:
    const size_t kPortionRequestSize = 1000;

private:
    sqlite3 *mDataBase = nullptr;
    string mTableName;
    Logger *mLog;
};


#endif //GEO_NETWORK_CLIENT_HISTORYSTORAGE_H
