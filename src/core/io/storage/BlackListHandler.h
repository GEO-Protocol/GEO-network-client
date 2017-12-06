#ifndef GEO_NETWORK_CLIENT_BACKLISTHANDLER_H
#define GEO_NETWORK_CLIENT_BACKLISTHANDLER_H

#include "../../common/NodeUUID.h"
#include "../../common/exceptions/IOError.h"

#include "../../logger/Logger.h"
#include "../../../libs/sqlite3/sqlite3.h"

#include <list>
#include <vector>
#include <memory>
#include <algorithm>


/**
 * @brief BackListHandler class
 *
 * Migrations handler collects migrations, that must be applied to the storage.
 * For the MVP release, SQLite plays the role if the internal storage,
 * but in the decentralised version - separate blockchain will be used.
 *
 * Migrations are used for sequential storage updates.
 */
class BlackListHandler {
public:
    BlackListHandler(
        sqlite3 *dbConnection,
        const string &tableName,
        Logger &logger);

public:
    vector<NodeUUID> allNodesUUIDS();

    void addNode(
        const NodeUUID &nodeUUID);

    bool checkIfNodeExists(
        const NodeUUID &contractorNode);

    void removeNodeFromBlackList(
        const NodeUUID &contractorNode);

protected:

    void ensureBlackListTable();

protected:
    LoggerStream info() const;

    LoggerStream debug() const;

    LoggerStream error() const;

    const string logHeader() const;

protected:
    Logger &mLog;
    sqlite3 *mDataBase;

    // This table is used for storing information about banned users.
    string mTableName;
};

#endif //GEO_NETWORK_CLIENT_BACKLISTHANDLER_H
