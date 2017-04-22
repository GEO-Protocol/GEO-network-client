#ifndef GEO_NETWORK_CLIENT_TRUSTLINEHANDLER_H
#define GEO_NETWORK_CLIENT_TRUSTLINEHANDLER_H

#include "../../common/NodeUUID.h"
#include "../../common/Types.h"
#include "../../trust_lines/TrustLine.h"
#include "../../logger/Logger.h"
#include "../../common/exceptions/IOError.h"
#include "../../common/multiprecision/MultiprecisionUtils.h"

#include "../../../libs/sqlite3/sqlite3.h"

#include <vector>

class TrustLineHandler {

public:

    TrustLineHandler(
        sqlite3 *dbConnection,
        const string &tableName,
        Logger *logger);

    void commit();

    void rollBack();

    void saveTrustLine(
        TrustLine::Shared trustLine);

    vector<TrustLine::Shared> allTrustLines ();

    void deleteTrustLine(const NodeUUID &contractorUUID);

private:

    void prepareInserted();

    bool containsContractor(const NodeUUID &contractorUUID);

    LoggerStream info() const;

    LoggerStream error() const;

    const string logHeader() const;

private:

    sqlite3 *mDataBase = nullptr;
    string mTableName;
    Logger *mLog;
    bool isTransactionBegin;

};


#endif //GEO_NETWORK_CLIENT_TRUSTLINEHANDLER_H
