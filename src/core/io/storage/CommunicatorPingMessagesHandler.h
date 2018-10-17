#ifndef GEO_NETWORK_CLIENT_COMMUNICATORPINGMESSAGESHANDLER_H
#define GEO_NETWORK_CLIENT_COMMUNICATORPINGMESSAGESHANDLER_H

#include "../../logger/Logger.h"
#include "../../common/Types.h"
#include "../../common/exceptions/IOError.h"

#include "../../../libs/sqlite3/sqlite3.h"

class CommunicatorPingMessagesHandler {

public:
    CommunicatorPingMessagesHandler(
        sqlite3 *dbConnection,
        const string &tableName,
        Logger &logger);

    void saveRecord(
        const NodeUUID &contractorUUID);

    vector<NodeUUID> allContractors();

    void deleteRecord(
        const NodeUUID &contractorUUID);

private:
    LoggerStream info() const;

    LoggerStream warning() const;

    const string logHeader() const;

private:
    sqlite3 *mDataBase = nullptr;
    string mTableName;
    Logger &mLog;
};


#endif //GEO_NETWORK_CLIENT_COMMUNICATORPINGMESSAGESHANDLER_H
