#ifndef GEO_NETWORK_CLIENT_FEATURESHANDLER_H
#define GEO_NETWORK_CLIENT_FEATURESHANDLER_H

#include "../../common/Types.h"
#include "../../logger/Logger.h"
#include "../../common/memory/MemoryUtils.h"
#include "../../common/exceptions/IOError.h"
#include "../../common/exceptions/ValueError.h"
#include "../../../libs/sqlite3/sqlite3.h"

#include <string>

class FeaturesHandler {

public:
    FeaturesHandler(
        sqlite3 *dbConnection,
        const string &tableName,
        Logger &logger);

    void saveFeature(
        string featureName,
        string featureValue);

    string getFeature(
        string featureName);

private:
    LoggerStream info() const;

    LoggerStream warning() const;

    const string logHeader() const;

private:
    sqlite3 *mDataBase = nullptr;
    string mTableName;
    Logger &mLog;
};


#endif //GEO_NETWORK_CLIENT_FEATURESHANDLER_H
