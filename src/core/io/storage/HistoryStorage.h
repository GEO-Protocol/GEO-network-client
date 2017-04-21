#ifndef GEO_NETWORK_CLIENT_HISTORYSTORAGE_H
#define GEO_NETWORK_CLIENT_HISTORYSTORAGE_H

#include "../../common/NodeUUID.h"
#include "../../common/Types.h"
#include "../../trust_lines/TrustLine.h"
#include "../../logger/Logger.h"
#include "../../common/exceptions/IOError.h"
#include "../../common/multiprecision/MultiprecisionUtils.h"

#include "../../db/operations_history_storage/record/base/Record.h"
#include "../../db/operations_history_storage/record/payment/PaymentRecord.h"
#include "../../db/operations_history_storage/record/trust_line/TrustLineRecord.h"

#include "../../../libs/sqlite3/sqlite3.h"

#include <vector>

using namespace db::operations_history_storage;

class HistoryStorage {

public:

    HistoryStorage(
        sqlite3 *dbConnection,
        const string &paymentsTableName,
        const string &trustLinesTableName,
        Logger *logger);

    bool commit();

    void rollBack();

    void saveTrustLineRecord(
        TrustLineRecord::Shared trustLine);

    void savePaymentRecord(
        PaymentRecord::Shared trustLine);

    vector<TrustLineRecord::Shared> allTrustLineRecords();

    vector<PaymentRecord::Shared> allPaymentRecords();

    vector<Record::Shared> allRecords();

private:

    void prepareInserted();

    LoggerStream info() const;

    LoggerStream error() const;

    const string logHeader() const;

private:

    sqlite3 *mDataBase = nullptr;
    string mPaymentsTableName;
    string mTrustLinesTableName;
    Logger *mLog;
    bool isTransactionBegin;
};


#endif //GEO_NETWORK_CLIENT_HISTORYSTORAGE_H
