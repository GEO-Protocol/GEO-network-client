#include "PathsManager.h"

PathsManager::PathsManager(
    TrustLinesManager *trustLinesManager,
    StorageHandler *storageHandler,
    Logger *logger):

    mTrustLinesManager(trustLinesManager),
    mStorageHandler(storageHandler),
    mLog(logger) {

    loadSelfRoutingTables();
}

void PathsManager::loadSelfRoutingTables() {

    mRT1 = mTrustLinesManager->rt1();
    mRT2 = mStorageHandler->routingTablesHandler()->routingTable2Level()->routeRecords();
    mRT3 = mStorageHandler->routingTablesHandler()->routingTable3Level()->routeRecords();
    info() << "Routing tables were loaded successfully";
    info() << "RT1 size: " << mRT1.size();
    info() << "RT2 size: " << mRT2.size();
    info() << "RT3 size: " << mRT3.size();

}

void PathsManager::removeUselessRecords() {

    set<NodeUUID> uselessNodes;
    auto itRt1Record = mRT1.begin();
    while (itRt1Record != mRT1.end()) {
        if (itRt1Record->second == TrustLineDirection::Incoming) {
            uselessNodes.insert(itRt1Record->first);
            //mRT1.erase(itRt1Record);
        } else {
            itRt1Record++;
        }
    }

    auto itRt2Record = mRT2.begin();
    while (itRt2Record != mRT2.end()) {
        NodeUUID source;
        NodeUUID target;
        TrustLineDirection direction;
        std::tie(source, target, direction) = *itRt2Record;
        if (uselessNodes.find(source) != uselessNodes.end()) {
            mRT2.erase(itRt2Record);
        } else {
            itRt2Record++;
        }
    }
}

LoggerStream PathsManager::info() const {

    if (nullptr == mLog)
        throw Exception("logger is not initialised");

    return mLog->info(logHeader());
}

const string PathsManager::logHeader() const {

    stringstream s;
    s << "[PathsManager]";

    return s.str();
}
