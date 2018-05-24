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
        TrustLineRecord::Shared record,
        const SerializedEquivalent equivalent);

    void savePaymentRecord(
        PaymentRecord::Shared record,
        const SerializedEquivalent equivalent);

    vector<TrustLineRecord::Shared> allTrustLineRecords(
        const SerializedEquivalent equivalent,
        size_t recordsCount,
        size_t fromRecord,
        DateTime timeFrom,
        bool isTimeFromPresent,
        DateTime timeTo,
        bool isTimeToPresent);

    vector<PaymentRecord::Shared> allPaymentRecords(
        const SerializedEquivalent equivalent,
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

    vector<PaymentRecord::Shared> allPaymentAdditionalRecords(
        const SerializedEquivalent equivalent,
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

    vector<Record::Shared> recordsWithContractor(
        const NodeUUID &contractorUUID,
        const SerializedEquivalent equivalent,
        size_t recordsCount,
        size_t fromRecord);

    bool whetherOperationWasConducted(
        const TransactionUUID &transactionUUID);

private:
    void savePaymentMainOutgoingRecord(
        PaymentRecord::Shared record,
        const SerializedEquivalent equivalent);

    void savePaymentMainIncomingRecord(
        PaymentRecord::Shared record,
        const SerializedEquivalent equivalent);

    void savePaymentAdditionalRecord(
        PaymentRecord::Shared record,
        const SerializedEquivalent equivalent);

    vector<PaymentRecord::Shared> allPaymentRecords(
        const SerializedEquivalent equivalent,
        size_t recordsCount,
        size_t fromRecord,
        DateTime timeFrom,
        bool isTimeFromPresent,
        DateTime timeTo,
        bool isTimeToPresent);

    vector<PaymentRecord::Shared> allPaymentAdditionalRecords(
        const SerializedEquivalent equivalent,
        size_t recordsCount,
        size_t fromRecord,
        DateTime timeFrom,
        bool isTimeFromPresent,
        DateTime timeTo,
        bool isTimeToPresent);

    size_t countRecordsByType(
        Record::RecordType recordType,
        const SerializedEquivalent equivalent);

    vector<Record::Shared> recordsPortionWithContractor(
        const NodeUUID &contractorUUID,
        const SerializedEquivalent equivalent,
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

    LoggerStream warning() const;

    const string logHeader() const;

private:
    const size_t kPortionRequestSize = 1000;

private:
    sqlite3 *mDataBase = nullptr;
    // main table used for storing history, needed for frontend
    // (trustlines, payments coordinator, payments receiver)
    string mMainTableName;
    // additional table used for storing history, needed for statistics
    // (cycles and payment intermediate nodes)
    string mAdditionalTableName;
    Logger &mLog;
};


#endif //GEO_NETWORK_CLIENT_HISTORYSTORAGE_H
