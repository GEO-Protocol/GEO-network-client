#ifndef GEO_NETWORK_CLIENT_HISTORYSTORAGE_H
#define GEO_NETWORK_CLIENT_HISTORYSTORAGE_H

#include "../../common/NodeUUID.h"
#include "../../common/Types.h"
#include "../../logger/Logger.h"
#include "../../common/exceptions/IOError.h"
#include "../../common/multiprecision/MultiprecisionUtils.h"

#include "record/base/Record.h"
#include "record/payment/PaymentRecord.h"
#include "record/trust_line/TrustLineRecord.h"

#include "../../../libs/sqlite3/sqlite3.h"

#include <vector>

class HistoryStorage {

public:

    HistoryStorage(
        sqlite3 *dbConnection,
        const string &tableName,
        Logger *logger);

    void commit();

    void rollBack();

    void saveRecord(
        Record::Shared record);

    vector<pair<TrustLineRecord::Shared, DateTime>> allTrustLineRecords(
        size_t recordsCount,
        size_t fromRecord);

    vector<pair<PaymentRecord::Shared, DateTime>> allPaymentRecords(
        size_t recordsCount,
        size_t fromRecord);

private:

    void prepareInserted();

    LoggerStream info() const;

    LoggerStream error() const;

    const string logHeader() const;

private:

    sqlite3 *mDataBase = nullptr;
    string mTableName;
    Logger *mLog;
    bool isTransactionBegin;
};


#endif //GEO_NETWORK_CLIENT_HISTORYSTORAGE_H
