#ifndef GEO_NETWORK_CLIENT_AUDITHANDLER_H
#define GEO_NETWORK_CLIENT_AUDITHANDLER_H

#include "../../logger/Logger.h"
#include "../../common/Types.h"
#include "../../common/multiprecision/MultiprecisionUtils.h"
#include "../../common/exceptions/IOError.h"

#include "../../../libs/sqlite3/sqlite3.h"

class AuditHandler {

public:
    AuditHandler(
        sqlite3 *dbConnection,
        const string &tableName,
        Logger &logger);

    void saveAudit(
        uint32_t id,
        uint32_t TrustLineID,
        uint32_t ownKeyHash,
        BytesShared ownSign,
        size_t ownSignBytesCount,
        uint32_t contractorKeyHash,
        BytesShared contractorSign,
        size_t contractorSignBytesCount,
        const TrustLineAmount &incomingAmount,
        const TrustLineAmount &outgoingAmount,
        const TrustLineBalance &balance);

private:
    LoggerStream info() const;

    LoggerStream warning() const;

    const string logHeader() const;

private:
    sqlite3 *mDataBase = nullptr;
    string mTableName;
    Logger &mLog;
};


#endif //GEO_NETWORK_CLIENT_AUDITHANDLER_H
