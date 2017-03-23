#include "StorageHandler.h"

StorageHandler::StorageHandler(
    const string &directory,
    const string &dataBaseName,
    Logger *logger):

    mDirectory(directory),
    mDataBaseName(dataBaseName),
    mDataBasePath(mDirectory + "/" + dataBaseName),
    mLog(logger) {

    checkDirectory();
    openConnection();
    mRoutingTablesHandler = new RoutingTablesHandler(
        mDataBase,
        mLog);
}

void StorageHandler::openConnection() {
    int rc = sqlite3_open_v2(mDataBasePath.c_str(), &mDataBase, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
    if (rc == SQLITE_OK) {
        info() << "Opened database successfully";
    } else {
        error() << "Can't open database " << kDataBaseName;
    }
}

StorageHandler::~StorageHandler() {
    closeConnection();
    delete mRoutingTablesHandler;
}

void StorageHandler::insertRT2(
    const NodeUUID &leftNode,
    const NodeUUID &rightNode,
    RoutingTableHandler::DirectionType direction) const {

    mRoutingTablesHandler->routingTable2Level()->insert(
        leftNode,
        rightNode,
        direction);
}

void StorageHandler::insertRT3(
        const NodeUUID &leftNode,
        const NodeUUID &rightNode,
        RoutingTableHandler::DirectionType direction) const {

    mRoutingTablesHandler->routingTable3Level()->insert(
        leftNode,
        rightNode,
        direction);
}

void StorageHandler::commit() const {

    mRoutingTablesHandler->routingTable2Level()->commit();
    mRoutingTablesHandler->routingTable3Level()->commit();
}

void StorageHandler::rollback() const {

    mRoutingTablesHandler->routingTable2Level()->rollBack();
    mRoutingTablesHandler->routingTable3Level()->rollBack();
}

void StorageHandler::prepareInsertred() const {

    mRoutingTablesHandler->routingTable2Level()->prepareInsertred();
    mRoutingTablesHandler->routingTable3Level()->prepareInsertred();
}

RoutingTablesHandler* StorageHandler::routingTablesHandler() const {

    return mRoutingTablesHandler;
}

vector<NodeUUID> StorageHandler::leftNodesRT2() const {

    return mRoutingTablesHandler->routingTable2Level()->leftNodes();
}

vector<NodeUUID> StorageHandler::leftNodesRT3() const {

    return mRoutingTablesHandler->routingTable3Level()->leftNodes();
}

void StorageHandler::closeConnection() {
    sqlite3_close_v2(mDataBase);
    info() << "Connection closed";
}

void StorageHandler::checkDirectory() {

    if (!fs::is_directory(fs::path(mDirectory))){
        fs::create_directories(
            fs::path(mDirectory));
    }
}

LoggerStream StorageHandler::info() const {
    if (nullptr == mLog)
        throw Exception("logger is not initialised");
    return mLog->info(logHeader());
}

LoggerStream StorageHandler::error() const {
    if (nullptr == mLog)
        throw Exception("logger is not initialised");
    return mLog->error(logHeader());
}

const string StorageHandler::logHeader() const {
    stringstream s;
    s << "[StorageHandler]";
    return s.str();
}
