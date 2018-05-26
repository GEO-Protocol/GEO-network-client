#ifndef GEO_NETWORK_CLIENT_AUDITHANDLER_H
#define GEO_NETWORK_CLIENT_AUDITHANDLER_H

#include "../../logger/Logger.h"
#include "../../common/exceptions/IOError.h"

#include "../../../libs/sqlite3/sqlite3.h"

class AuditHandler {

public:
    AuditHandler(
        sqlite3 *dbConnection,
        const string &tableName,
        Logger &logger);

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
