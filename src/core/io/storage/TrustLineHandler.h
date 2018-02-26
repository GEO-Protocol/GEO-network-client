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

    [[deprecated("Use saving with equivalent instead.")]]
    void saveTrustLine(
        TrustLine::Shared trustLine);

    void saveTrustLine(
        TrustLine::Shared trustLine,
        const SerializedEquivalent equivalent);

    [[deprecated("Use allTrustLinesByEquivalent instead.")]]
    vector<TrustLine::Shared> allTrustLines();

    vector<TrustLine::Shared> allTrustLinesByEquivalent(
        const SerializedEquivalent equivalent);

    [[deprecated("Use deleting with equivalent instead.")]]
    void deleteTrustLine(const NodeUUID &contractorUUID);

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
