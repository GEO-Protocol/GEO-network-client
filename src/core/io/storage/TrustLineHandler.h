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
        Logger &logger);

    void saveTrustLine(
        TrustLine::Shared trustLine,
        const SerializedEquivalent equivalent);

    vector<TrustLine::Shared> allTrustLinesByEquivalent(
        const SerializedEquivalent equivalent);

    void deleteTrustLine(
        const NodeUUID &contractorUUID,
        const SerializedEquivalent equivalent);

    vector<SerializedEquivalent> equivalents();

    const string &tableName() const;

private:
    LoggerStream info() const;

    LoggerStream warning() const;

    const string logHeader() const;

private:
    sqlite3 *mDataBase = nullptr;
    string mTableName;
    Logger &mLog;
};


#endif //GEO_NETWORK_CLIENT_TRUSTLINEHANDLER_H
