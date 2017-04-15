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
        const string &dataBasePath,
        const string &tableName,
        Logger *logger);

    bool commit();

    void rollBack();

    void saveTrustLine(
        TrustLine::Shared trustLine);

    vector<TrustLine::Shared> trustLines();

    void deleteTrustLine(const NodeUUID &contractorUUID);

    void closeConnection();

private:

    void prepareInserted();

    void insert(
        TrustLine::Shared trustLine);

    void update(TrustLine::Shared trustLine);

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
