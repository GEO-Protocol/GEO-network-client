#ifndef GEO_NETWORK_CLIENT_ROUTINGTABLEHANDLER_H
#define GEO_NETWORK_CLIENT_ROUTINGTABLEHANDLER_H

#include "../../common/NodeUUID.h"
#include "../../logger/Logger.h"

#include "../../../libs/sqlite3/sqlite3.h"

class RoutingTableHandler {

public:

    enum DirectionType {
        Incoming = 1,
        Outgoing,
        Both
    };

public:

    RoutingTableHandler(
        sqlite3 *db,
        string tableName,
        Logger *logger);

    void prepareInsertred();

    void commit();

    void rollBack();

    void insert(
        const NodeUUID &leftNode,
        const NodeUUID &rightNode,
        DirectionType directionType);

    vector<NodeUUID> leftNodes();

private:

    const string createTableQuery() const;

    const string createIndexQuery(string fieldName) const;

    const string insertHeaderQuery() const;

    const string insertBodyQuery() const;

    const string selectQuery() const;

    LoggerStream info() const;

    LoggerStream error() const;

    const string logHeader() const;

private:

    sqlite3 *mDataBase;
    string mTableName;
    sqlite3_stmt *stmt;
    Logger *mLog;
    vector<NodeUUID> mLeftNodes;
    vector<NodeUUID> mRightNodes;
    vector<DirectionType> mDirections;

};


#endif //GEO_NETWORK_CLIENT_ROUTINGTABLEHANDLER_H
