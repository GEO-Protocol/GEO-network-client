#include "PathsManager.h"

PathsManager::PathsManager(
    const NodeUUID &nodeUUID,
    TrustLinesManager *trustLinesManager,
    StorageHandler *storageHandler,
    Logger *logger):

    mNodeUUID(nodeUUID),
    mTrustLinesManager(trustLinesManager),
    mStorageHandler(storageHandler),
    mLog(logger),
    mPathCollection(nullptr){

    fillRoutingTables();
}

void PathsManager::setContractorRoutingTables(ResultRoutingTablesMessage::Shared message) {

    contractorUUID = message->senderUUID();
    contractorRT1 = message->rt1();
    contractorRT2 = message->rt2();
    contractorRT3 = message->rt3();
}

void PathsManager::findDirectPath() {

    vector<NodeUUID> intermediateNodes;
    for (auto &nodeAndDirection : mTrustLinesManager->rt1()) {
        if (nodeAndDirection.first == contractorUUID) {
            Path path = Path(
                mNodeUUID,
                contractorUUID,
                intermediateNodes);
            mPathCollection->add(path);
            info() << "found direct path";
            return;
        }
    }
}

void PathsManager::findPathOnSecondLevel() {

    for (auto &nodeUUID : mStorageHandler->routingTablesHandler()->subRoutesSecondLevel(contractorUUID)) {
        vector<NodeUUID> intermediateNodes;
        intermediateNodes.push_back(nodeUUID);
        Path path(
            mNodeUUID,
            contractorUUID,
            intermediateNodes);
        mPathCollection->add(path);
        info() << "found path on second level";
    }
}

void PathsManager::findPathOnThirdLevel() {

    for (auto &nodeUUIDAndNodeUUID : mStorageHandler->routingTablesHandler()->subRoutesThirdLevel(contractorUUID)) {
        vector<NodeUUID> intermediateNodes;
        intermediateNodes.push_back(nodeUUIDAndNodeUUID.first);
        intermediateNodes.push_back(nodeUUIDAndNodeUUID.second);
        Path path(
            mNodeUUID,
            contractorUUID,
            intermediateNodes);
        mPathCollection->add(path);
        info() << "found path on third level";
    }
}

Path::Shared PathsManager::findPath() {

    info() << "start finding paths to " << contractorUUID;
    if (mPathCollection != nullptr) {
        delete mPathCollection;
    }
    mPathCollection = new PathsCollection(
        mNodeUUID,
        contractorUUID);
    findDirectPath();
    findPathOnSecondLevel();
    findPathOnThirdLevel();

    info() << "total paths count: " << mPathCollection->count();
    while (mPathCollection->hasNextPath()) {
        info() << mPathCollection->nextPath()->toString();
    }
    mPathCollection->resetCurrentPath();
    if (mPathCollection->hasNextPath()) {
        return mPathCollection->nextPath();
    } else {
        return nullptr;
    }
}

PathsCollection* PathsManager::pathCollection() const {

    return mPathCollection;
}

void PathsManager::fillRoutingTables() {

    NodeUUID* nodeUUID90Ptr = new NodeUUID("13e5cf8c-5834-4e52-b65b-f9281dd1ff90");
    NodeUUID* nodeUUID91Ptr = new NodeUUID("13e5cf8c-5834-4e52-b65b-f9281dd1ff91");
    NodeUUID* nodeUUID92Ptr = new NodeUUID("13e5cf8c-5834-4e52-b65b-f9281dd1ff92");
    NodeUUID* nodeUUID93Ptr = new NodeUUID("13e5cf8c-5834-4e52-b65b-f9281dd1ff93");
    NodeUUID* nodeUUID94Ptr = new NodeUUID("13e5cf8c-5834-4e52-b65b-f9281dd1ff94");
    NodeUUID* nodeUUID95Ptr = new NodeUUID("13e5cf8c-5834-4e52-b65b-f9281dd1ff95");
    NodeUUID* nodeUUID96Ptr = new NodeUUID("13e5cf8c-5834-4e52-b65b-f9281dd1ff96");
    NodeUUID* nodeUUID97Ptr = new NodeUUID("13e5cf8c-5834-4e52-b65b-f9281dd1ff97");
    NodeUUID* nodeUUID98Ptr = new NodeUUID("13e5cf8c-5834-4e52-b65b-f9281dd1ff98");

    if (!mNodeUUID.stringUUID().compare("13e5cf8c-5834-4e52-b65b-f9281dd1ff90")) {
        mStorageHandler->routingTablesHandler()->routingTable2Level()->prepareInsertred();
        mStorageHandler->routingTablesHandler()->routingTable2Level()->insert(*nodeUUID92Ptr, *nodeUUID94Ptr,
                                                                              TrustLineDirection::Both);
        mStorageHandler->routingTablesHandler()->routingTable2Level()->insert(*nodeUUID94Ptr, *nodeUUID92Ptr,
                                                                              TrustLineDirection::Both);
        mStorageHandler->routingTablesHandler()->routingTable2Level()->insert(*nodeUUID94Ptr, *nodeUUID96Ptr,
                                                                              TrustLineDirection::Both);
        mStorageHandler->routingTablesHandler()->routingTable2Level()->insert(*nodeUUID94Ptr, *nodeUUID97Ptr,
                                                                              TrustLineDirection::Both);
        mStorageHandler->routingTablesHandler()->routingTable2Level()->insert(*nodeUUID94Ptr, *nodeUUID98Ptr,
                                                                              TrustLineDirection::Both);
        mStorageHandler->routingTablesHandler()->routingTable2Level()->commit();

        mStorageHandler->routingTablesHandler()->routingTable3Level()->prepareInsertred();
        mStorageHandler->routingTablesHandler()->routingTable3Level()->insert(*nodeUUID94Ptr, *nodeUUID90Ptr,
                                                                              TrustLineDirection::Both);
        mStorageHandler->routingTablesHandler()->routingTable3Level()->insert(*nodeUUID94Ptr, *nodeUUID96Ptr,
                                                                              TrustLineDirection::Both);
        mStorageHandler->routingTablesHandler()->routingTable3Level()->insert(*nodeUUID94Ptr, *nodeUUID97Ptr,
                                                                              TrustLineDirection::Both);
        mStorageHandler->routingTablesHandler()->routingTable3Level()->insert(*nodeUUID94Ptr, *nodeUUID98Ptr,
                                                                              TrustLineDirection::Both);
        mStorageHandler->routingTablesHandler()->routingTable3Level()->insert(*nodeUUID96Ptr, *nodeUUID95Ptr,
                                                                              TrustLineDirection::Both);
        mStorageHandler->routingTablesHandler()->routingTable3Level()->insert(*nodeUUID98Ptr, *nodeUUID95Ptr,
                                                                              TrustLineDirection::Both);
        mStorageHandler->routingTablesHandler()->routingTable3Level()->commit();
    }

    if (!mNodeUUID.stringUUID().compare("13e5cf8c-5834-4e52-b65b-f9281dd1ff91")) {
        mStorageHandler->routingTablesHandler()->routingTable2Level()->prepareInsertred();
        mStorageHandler->routingTablesHandler()->routingTable2Level()->insert(*nodeUUID93Ptr, *nodeUUID95Ptr,
                                                                              TrustLineDirection::Both);
        mStorageHandler->routingTablesHandler()->routingTable2Level()->commit();

        mStorageHandler->routingTablesHandler()->routingTable3Level()->prepareInsertred();
        mStorageHandler->routingTablesHandler()->routingTable3Level()->insert(*nodeUUID95Ptr, *nodeUUID96Ptr,
                                                                              TrustLineDirection::Both);
        mStorageHandler->routingTablesHandler()->routingTable3Level()->insert(*nodeUUID95Ptr, *nodeUUID98Ptr,
                                                                              TrustLineDirection::Both);
        mStorageHandler->routingTablesHandler()->routingTable3Level()->commit();
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
