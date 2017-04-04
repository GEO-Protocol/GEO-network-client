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

#ifdef STORAGE_HANDLER_DEBUG_LOG
    testStorageHandler();
#endif

    // TODO remove from here
    //testStorageHandler();
    //fillRoutingTables();
    //fillBigRoutingTables();
    //testTrustLineHandler();
}

void PathsManager::setContractorRoutingTables(ResultRoutingTablesMessage::Shared message) {

    contractorUUID = message->senderUUID();
    contractorRT1 = message->rt1();
    contractorRT2 = message->rt2();
    contractorRT3 = message->rt3();
}

void PathsManager::findDirectPath() {

    for (auto const &nodeUUID : mTrustLinesManager->rt1()) {
        if (nodeUUID == contractorUUID) {
            Path path = Path(
                mNodeUUID,
                contractorUUID);
            mPathCollection->add(path);
            info() << "found direct path";
            return;
        }
    }
}

void PathsManager::findPathsOnSecondLevel() {

    for (auto const &nodeUUID : mStorageHandler->routingTablesHandler()->subRoutesSecondLevel(contractorUUID)) {
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

void PathsManager::findPathsOnThirdLevel() {

    for (auto const &nodeUUIDAndNodeUUID : mStorageHandler->routingTablesHandler()->subRoutesThirdLevelContractor(
            contractorUUID,
            mNodeUUID)) {
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

void PathsManager::findPathsOnForthLevel() {

    for (auto const &nodeUUID : contractorRT1) {
        if (nodeUUID == mNodeUUID) {
            continue;
        }
        for (auto const &nodeUUIDAndNodeUUID : mStorageHandler->routingTablesHandler()->subRoutesThirdLevelWithForbiddenNodes(
                nodeUUID,
                mNodeUUID,
                contractorUUID)) {
            vector<NodeUUID> intermediateNodes;
            intermediateNodes.push_back(nodeUUIDAndNodeUUID.first);
            intermediateNodes.push_back(nodeUUIDAndNodeUUID.second);
            intermediateNodes.push_back(nodeUUID);
            info() << "forth level: " << nodeUUIDAndNodeUUID.first << " " << nodeUUIDAndNodeUUID.second << " " << nodeUUID;
            Path path(
                mNodeUUID,
                contractorUUID,
                intermediateNodes);
            mPathCollection->add(path);
            info() << "found path on forth level";
        }
    }
}

void PathsManager::findPathsOnFifthLevel() {

    for (auto const &itRT2 : contractorRT2) {
        // TODO (mc) : need or not second condition (itRT2.first == contractorUUID)
        if (itRT2.first == mNodeUUID || itRT2.first == contractorUUID) {
            continue;
        }
        for (auto const &nodeUUIDAndNodeUUID : mStorageHandler->routingTablesHandler()->subRoutesThirdLevelWithForbiddenNodes(
                itRT2.first,
                mNodeUUID,
                contractorUUID)) {
            vector<NodeUUID> intermediateNodes;
            intermediateNodes.push_back(nodeUUIDAndNodeUUID.first);
            intermediateNodes.push_back(nodeUUIDAndNodeUUID.second);
            intermediateNodes.push_back(itRT2.first);
            info() << "fifth path: " << itRT2.first << " " << nodeUUIDAndNodeUUID.first << " " << nodeUUIDAndNodeUUID.second;
            for (auto const &nodeUUID : itRT2.second) {
                if(std::find(intermediateNodes.begin(), intermediateNodes.end(), nodeUUID) != intermediateNodes.end() ||
                        nodeUUID == mNodeUUID || nodeUUID == contractorUUID) {
                    continue;
                }
                intermediateNodes.push_back(nodeUUID);
                Path path(
                    mNodeUUID,
                    contractorUUID,
                    intermediateNodes);
                mPathCollection->add(path);
                intermediateNodes.pop_back();
                info() << "found path on fifth level";
            }
        }
    }
}

void PathsManager::findPathsOnSixthLevel() {

    /*info() << "sixth path RT3: ";
    for (auto const &itMap : contractorRT3) {
        info() << "key: " << itMap.first;
        for (auto const &itVector : itMap.second) {
            info() << "\tvalue: " << itVector;
        }
    }*/

    for (auto const &itRT3 : contractorRT3) {
        // TODO (mc) : need or not second condition (itRT3.first == contractorUUID)
        if (itRT3.first == mNodeUUID || itRT3.first == contractorUUID) {
            continue;
        }
        for (auto const &nodeUUIDAndNodeUUID : mStorageHandler->routingTablesHandler()->subRoutesThirdLevelWithForbiddenNodes(
                itRT3.first,
                mNodeUUID,
                contractorUUID)) {
            vector<NodeUUID> intermediateNodes;
            intermediateNodes.push_back(nodeUUIDAndNodeUUID.first);
            intermediateNodes.push_back(nodeUUIDAndNodeUUID.second);
            intermediateNodes.push_back(itRT3.first);
            //info() << "sixth path: " << itRT3.first << " " << nodeUUIDAndNodeUUID.second << " " << nodeUUIDAndNodeUUID.first;
            for (auto const &nodeUUID : itRT3.second) {
                for (auto &contactorIntermediateNode : intermediateNodesOnContractorFirstLevel(
                        nodeUUID,
                        intermediateNodes)) {
                    //info() << "sixth path: " << nodeUUID << " " << contactorIntermediateNode;
                    intermediateNodes.push_back(nodeUUID);
                    intermediateNodes.push_back(contactorIntermediateNode);
                    Path path(
                        mNodeUUID,
                        contractorUUID,
                        intermediateNodes);
                    mPathCollection->add(path);
                    intermediateNodes.pop_back();
                    intermediateNodes.pop_back();
                    info() << "found path on sixth level";
                }
            }
        }
    }
}

vector<NodeUUID> PathsManager::intermediateNodesOnContractorFirstLevel(
        const NodeUUID &thirdLevelSourceNode,
        const vector<NodeUUID> intermediateNodes) const {

    if(std::find(intermediateNodes.begin(), intermediateNodes.end(), thirdLevelSourceNode) != intermediateNodes.end() ||
            thirdLevelSourceNode == mNodeUUID || thirdLevelSourceNode == contractorUUID) {
        return {};
    }

    auto nodeUUIDAndVect = contractorRT2.find(thirdLevelSourceNode);
    if (nodeUUIDAndVect == contractorRT2.end()) {
        return {};
    } else {
        vector<NodeUUID> result;
        for (auto const &nodeUUID : nodeUUIDAndVect->second) {
            if(std::find(intermediateNodes.begin(), intermediateNodes.end(), nodeUUID) != intermediateNodes.end() ||
                    nodeUUID == mNodeUUID || nodeUUID == contractorUUID) {
                continue;
            }
            result.push_back(nodeUUID);
        }
        return result;
    }
}

// test
void PathsManager::findPathsOnSecondLevelTest() {

    for (auto const &nodeUUID : mStorageHandler->routingTablesHandler()->subRoutesSecondLevel(contractorUUID)) {
        Path path(
            mNodeUUID,
            contractorUUID,
            {nodeUUID});
        mPathCollection->add(path);
        info() << "found path on second level";
    }
}

void PathsManager::findPathsOnThirdLevelTest() {

    for (auto const &nodeUUIDAndNodeUUID : mStorageHandler->routingTablesHandler()->subRoutesThirdLevel(
            contractorUUID)) {
        if (nodeUUIDAndNodeUUID.first == contractorUUID || nodeUUIDAndNodeUUID.second == mNodeUUID) {
            continue;
        }
        Path path(
            mNodeUUID,
            contractorUUID,
            {
                nodeUUIDAndNodeUUID.first,
                nodeUUIDAndNodeUUID.second});
        mPathCollection->add(path);
        info() << "found path on third level";
    }
}

void PathsManager::findPathsOnForthLevelTest() {

    for (auto const &nodeUUID : contractorRT1) {
        if (nodeUUID == mNodeUUID) {
            continue;
        }
        for (auto const &nodeUUIDAndNodeUUID : mStorageHandler->routingTablesHandler()->subRoutesThirdLevel(
                nodeUUID)) {
            Path path(
                mNodeUUID,
                contractorUUID,
                {
                    nodeUUIDAndNodeUUID.first,
                    nodeUUIDAndNodeUUID.second,
                    nodeUUID});
            mPathCollection->add(path);
            info() << "found path on forth level";
        }
    }
}

void PathsManager::findPathsOnFifthLevelTest() {

    for (auto const &itRT2 : contractorRT2) {
        // TODO (mc) : need or not second condition (itRT2.first == contractorUUID)
        if (itRT2.first == mNodeUUID || itRT2.first == contractorUUID) {
            continue;
        }
        for (auto const &nodeUUIDAndNodeUUID : mStorageHandler->routingTablesHandler()->subRoutesThirdLevel(
                itRT2.first)) {
            info() << "fifth path: " << itRT2.first << " " << nodeUUIDAndNodeUUID.first << " " << nodeUUIDAndNodeUUID.second;
            for (auto const &nodeUUID : itRT2.second) {
                vector<NodeUUID> intermediateNodes;
                intermediateNodes.push_back(nodeUUIDAndNodeUUID.first);
                intermediateNodes.push_back(nodeUUIDAndNodeUUID.second);
                intermediateNodes.push_back(itRT2.first);
                intermediateNodes.push_back(nodeUUID);
                Path path(
                    mNodeUUID,
                    contractorUUID,
                    intermediateNodes);
                mPathCollection->add(path);
                info() << "found path on fifth level";
            }
        }
    }
}

void PathsManager::findPathsOnSixthLevelTest() {

    for (auto const &itRT3 : contractorRT3) {
        // TODO (mc) : need or not second condition (itRT3.first == contractorUUID)
        if (itRT3.first == mNodeUUID || itRT3.first == contractorUUID) {
            continue;
        }
        for (auto const &nodeUUIDAndNodeUUID : mStorageHandler->routingTablesHandler()->subRoutesThirdLevel(
                itRT3.first)) {

            info() << "sixth path: " << itRT3.first << " " << nodeUUIDAndNodeUUID.second << " " << nodeUUIDAndNodeUUID.first;
            for (auto const &nodeUUID : itRT3.second) {
                for (auto &contactorIntermediateNode : intermediateNodesOnContractorFirstLevelTest(
                        nodeUUID)) {
                    info() << "sixth path: " << nodeUUID << " " << contactorIntermediateNode;
                    vector<NodeUUID> intermediateNodes;
                    intermediateNodes.push_back(nodeUUIDAndNodeUUID.first);
                    intermediateNodes.push_back(nodeUUIDAndNodeUUID.second);
                    intermediateNodes.push_back(itRT3.first);
                    intermediateNodes.push_back(nodeUUID);
                    intermediateNodes.push_back(contactorIntermediateNode);
                    Path path(
                        mNodeUUID,
                        contractorUUID,
                        intermediateNodes);
                    mPathCollection->add(path);
                    info() << "found path on sixth level";
                }
            }
        }
    }
}

vector<NodeUUID> PathsManager::intermediateNodesOnContractorFirstLevelTest(
        const NodeUUID &thirdLevelSourceNode) const {

    auto nodeUUIDAndVect = contractorRT2.find(thirdLevelSourceNode);
    if (nodeUUIDAndVect == contractorRT2.end()) {
        return {};
    } else {
        return nodeUUIDAndVect->second;
    }
}
// test end

Path::Shared PathsManager::findPath() {

    info() << "start finding paths to " << contractorUUID;
    if (mPathCollection != nullptr) {
        delete mPathCollection;
    }
    mPathCollection = new PathsCollection(
        mNodeUUID,
        contractorUUID);
    findDirectPath();
    findPathsOnSecondLevel();
    findPathsOnThirdLevel();
    findPathsOnForthLevel();
    findPathsOnFifthLevel();
    findPathsOnSixthLevel();

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

Path::Shared PathsManager::findPathsTest() {

    info() << "start finding paths to " << contractorUUID;
    if (mPathCollection != nullptr) {
        delete mPathCollection;
    }
    mPathCollection = new PathsCollection(
            mNodeUUID,
            contractorUUID);
    findDirectPath();
    findPathsOnSecondLevelTest();
    findPathsOnThirdLevelTest();
    findPathsOnForthLevelTest();
    findPathsOnFifthLevelTest();
    findPathsOnSixthLevelTest();

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
        mStorageHandler->routingTablesHandler()->routingTable2Level()->insert(*nodeUUID92Ptr, *nodeUUID94Ptr,
                                                                              TrustLineDirection::Both);
        // duplicate
        /*mStorageHandler->routingTablesHandler()->routingTable2Level()->insert(*nodeUUID92Ptr, *nodeUUID94Ptr,
                                                                              TrustLineDirection::Both);*/
        mStorageHandler->routingTablesHandler()->routingTable2Level()->insert(*nodeUUID92Ptr, *nodeUUID90Ptr,
                                                                              TrustLineDirection::Both);
        mStorageHandler->routingTablesHandler()->routingTable2Level()->insert(*nodeUUID94Ptr, *nodeUUID92Ptr,
                                                                              TrustLineDirection::Both);
        mStorageHandler->routingTablesHandler()->routingTable2Level()->insert(*nodeUUID94Ptr, *nodeUUID96Ptr,
                                                                              TrustLineDirection::Both);
        mStorageHandler->routingTablesHandler()->routingTable2Level()->insert(*nodeUUID94Ptr, *nodeUUID97Ptr,
                                                                              TrustLineDirection::Both);
        mStorageHandler->routingTablesHandler()->routingTable2Level()->insert(*nodeUUID94Ptr, *nodeUUID98Ptr,
                                                                              TrustLineDirection::Both);
        mStorageHandler->routingTablesHandler()->routingTable2Level()->insert(*nodeUUID94Ptr, *nodeUUID90Ptr,
                                                                              TrustLineDirection::Both);
        mStorageHandler->routingTablesHandler()->routingTable2Level()->commit();

        mStorageHandler->routingTablesHandler()->routingTable3Level()->insert(*nodeUUID94Ptr, *nodeUUID90Ptr,
                                                                              TrustLineDirection::Both);
        mStorageHandler->routingTablesHandler()->routingTable3Level()->insert(*nodeUUID94Ptr, *nodeUUID92Ptr,
                                                                              TrustLineDirection::Both);
        mStorageHandler->routingTablesHandler()->routingTable3Level()->insert(*nodeUUID94Ptr, *nodeUUID96Ptr,
                                                                              TrustLineDirection::Both);
        mStorageHandler->routingTablesHandler()->routingTable3Level()->insert(*nodeUUID94Ptr, *nodeUUID97Ptr,
                                                                              TrustLineDirection::Both);
        mStorageHandler->routingTablesHandler()->routingTable3Level()->insert(*nodeUUID94Ptr, *nodeUUID98Ptr,
                                                                              TrustLineDirection::Both);
        mStorageHandler->routingTablesHandler()->routingTable3Level()->insert(*nodeUUID96Ptr, *nodeUUID95Ptr,
                                                                              TrustLineDirection::Both);
        mStorageHandler->routingTablesHandler()->routingTable3Level()->insert(*nodeUUID96Ptr, *nodeUUID94Ptr,
                                                                              TrustLineDirection::Both);
        mStorageHandler->routingTablesHandler()->routingTable3Level()->insert(*nodeUUID98Ptr, *nodeUUID95Ptr,
                                                                              TrustLineDirection::Both);
        mStorageHandler->routingTablesHandler()->routingTable3Level()->insert(*nodeUUID98Ptr, *nodeUUID94Ptr,
                                                                              TrustLineDirection::Both);
        mStorageHandler->routingTablesHandler()->routingTable3Level()->insert(*nodeUUID97Ptr, *nodeUUID94Ptr,
                                                                              TrustLineDirection::Both);
        mStorageHandler->routingTablesHandler()->routingTable3Level()->insert(*nodeUUID92Ptr, *nodeUUID90Ptr,
                                                                              TrustLineDirection::Both);
        mStorageHandler->routingTablesHandler()->routingTable3Level()->insert(*nodeUUID92Ptr, *nodeUUID94Ptr,
                                                                              TrustLineDirection::Both);
        mStorageHandler->routingTablesHandler()->routingTable3Level()->commit();
    }

    if (!mNodeUUID.stringUUID().compare("13e5cf8c-5834-4e52-b65b-f9281dd1ff91")) {
        mStorageHandler->routingTablesHandler()->routingTable2Level()->insert(*nodeUUID93Ptr, *nodeUUID95Ptr,
                                                                              TrustLineDirection::Both);
        mStorageHandler->routingTablesHandler()->routingTable2Level()->insert(*nodeUUID93Ptr, *nodeUUID91Ptr,
                                                                              TrustLineDirection::Both);
        mStorageHandler->routingTablesHandler()->routingTable2Level()->commit();

        mStorageHandler->routingTablesHandler()->routingTable3Level()->insert(*nodeUUID95Ptr, *nodeUUID96Ptr,
                                                                              TrustLineDirection::Both);
        mStorageHandler->routingTablesHandler()->routingTable3Level()->insert(*nodeUUID95Ptr, *nodeUUID98Ptr,
                                                                              TrustLineDirection::Both);
        mStorageHandler->routingTablesHandler()->routingTable3Level()->insert(*nodeUUID95Ptr, *nodeUUID93Ptr,
                                                                              TrustLineDirection::Both);
        mStorageHandler->routingTablesHandler()->routingTable3Level()->commit();
    }

    if (!mNodeUUID.stringUUID().compare("13e5cf8c-5834-4e52-b65b-f9281dd1ff94")) {
        mStorageHandler->routingTablesHandler()->routingTable2Level()->insert(*nodeUUID92Ptr, *nodeUUID90Ptr,
                                                                              TrustLineDirection::Both);
        mStorageHandler->routingTablesHandler()->routingTable2Level()->insert(*nodeUUID92Ptr, *nodeUUID94Ptr,
                                                                              TrustLineDirection::Both);
        mStorageHandler->routingTablesHandler()->routingTable2Level()->insert(*nodeUUID96Ptr, *nodeUUID95Ptr,
                                                                              TrustLineDirection::Both);
        mStorageHandler->routingTablesHandler()->routingTable2Level()->insert(*nodeUUID96Ptr, *nodeUUID94Ptr,
                                                                              TrustLineDirection::Both);
        mStorageHandler->routingTablesHandler()->routingTable2Level()->insert(*nodeUUID98Ptr, *nodeUUID95Ptr,
                                                                              TrustLineDirection::Both);
        mStorageHandler->routingTablesHandler()->routingTable2Level()->insert(*nodeUUID98Ptr, *nodeUUID94Ptr,
                                                                              TrustLineDirection::Both);
        mStorageHandler->routingTablesHandler()->routingTable2Level()->insert(*nodeUUID97Ptr, *nodeUUID94Ptr,
                                                                              TrustLineDirection::Both);
        mStorageHandler->routingTablesHandler()->routingTable2Level()->commit();

        mStorageHandler->routingTablesHandler()->routingTable3Level()->insert(*nodeUUID90Ptr, *nodeUUID92Ptr,
                                                                              TrustLineDirection::Both);
        mStorageHandler->routingTablesHandler()->routingTable3Level()->insert(*nodeUUID90Ptr, *nodeUUID94Ptr,
                                                                              TrustLineDirection::Both);
        mStorageHandler->routingTablesHandler()->routingTable3Level()->insert(*nodeUUID92Ptr, *nodeUUID90Ptr,
                                                                              TrustLineDirection::Both);
        mStorageHandler->routingTablesHandler()->routingTable3Level()->insert(*nodeUUID92Ptr, *nodeUUID94Ptr,
                                                                              TrustLineDirection::Both);
        mStorageHandler->routingTablesHandler()->routingTable3Level()->insert(*nodeUUID95Ptr, *nodeUUID93Ptr,
                                                                              TrustLineDirection::Both);
        mStorageHandler->routingTablesHandler()->routingTable3Level()->insert(*nodeUUID95Ptr, *nodeUUID98Ptr,
                                                                              TrustLineDirection::Both);
        mStorageHandler->routingTablesHandler()->routingTable3Level()->insert(*nodeUUID95Ptr, *nodeUUID96Ptr,
                                                                              TrustLineDirection::Both);
        mStorageHandler->routingTablesHandler()->routingTable3Level()->commit();
    }

    if (!mNodeUUID.stringUUID().compare("13e5cf8c-5834-4e52-b65b-f9281dd1ff93")) {
        mStorageHandler->routingTablesHandler()->routingTable2Level()->insert(*nodeUUID95Ptr, *nodeUUID96Ptr,
                                                                              TrustLineDirection::Both);
        mStorageHandler->routingTablesHandler()->routingTable2Level()->insert(*nodeUUID95Ptr, *nodeUUID98Ptr,
                                                                              TrustLineDirection::Both);
        mStorageHandler->routingTablesHandler()->routingTable2Level()->insert(*nodeUUID95Ptr, *nodeUUID93Ptr,
                                                                              TrustLineDirection::Both);
        mStorageHandler->routingTablesHandler()->routingTable2Level()->commit();

        mStorageHandler->routingTablesHandler()->routingTable3Level()->insert(*nodeUUID96Ptr, *nodeUUID94Ptr,
                                                                              TrustLineDirection::Both);
        mStorageHandler->routingTablesHandler()->routingTable3Level()->insert(*nodeUUID96Ptr, *nodeUUID95Ptr,
                                                                              TrustLineDirection::Both);
        mStorageHandler->routingTablesHandler()->routingTable3Level()->insert(*nodeUUID98Ptr, *nodeUUID94Ptr,
                                                                              TrustLineDirection::Both);
        mStorageHandler->routingTablesHandler()->routingTable3Level()->insert(*nodeUUID98Ptr, *nodeUUID95Ptr,
                                                                              TrustLineDirection::Both);
        mStorageHandler->routingTablesHandler()->routingTable3Level()->commit();
    }

    delete nodeUUID90Ptr;
    delete nodeUUID91Ptr;
    delete nodeUUID92Ptr;
    delete nodeUUID93Ptr;
    delete nodeUUID94Ptr;
    delete nodeUUID95Ptr;
    delete nodeUUID96Ptr;
    delete nodeUUID97Ptr;
    delete nodeUUID98Ptr;
}

void PathsManager::testStorageHandler() {

    NodeUUID* nodeUUID81Ptr = new NodeUUID("13e5cf8c-5834-4e52-b65b-f9281dd1ff81");
    NodeUUID* nodeUUID82Ptr = new NodeUUID("13e5cf8c-5834-4e52-b65b-f9281dd1ff82");
    NodeUUID* nodeUUID83Ptr = new NodeUUID("13e5cf8c-5834-4e52-b65b-f9281dd1ff83");
    NodeUUID* nodeUUID84Ptr = new NodeUUID("13e5cf8c-5834-4e52-b65b-f9281dd1ff84");
    NodeUUID* nodeUUID85Ptr = new NodeUUID("13e5cf8c-5834-4e52-b65b-f9281dd1ff85");
    NodeUUID* nodeUUID86Ptr = new NodeUUID("13e5cf8c-5834-4e52-b65b-f9281dd1ff86");
    mStorageHandler->routingTablesHandler()->routingTable2Level()->insert(*nodeUUID81Ptr, *nodeUUID82Ptr, TrustLineDirection::Both);
    mStorageHandler->routingTablesHandler()->routingTable2Level()->rollBack();
    mStorageHandler->routingTablesHandler()->routingTable2Level()->insert(*nodeUUID81Ptr, *nodeUUID82Ptr, TrustLineDirection::Both);
    mStorageHandler->routingTablesHandler()->routingTable2Level()->commit();
    mStorageHandler->routingTablesHandler()->routingTable2Level()->insert(*nodeUUID83Ptr, *nodeUUID84Ptr, TrustLineDirection::Incoming);
    mStorageHandler->routingTablesHandler()->routingTable2Level()->insert(*nodeUUID85Ptr, *nodeUUID86Ptr, TrustLineDirection::Outgoing);
    mStorageHandler->routingTablesHandler()->routingTable2Level()->commit();
    mStorageHandler->routingTablesHandler()->routingTable2Level()->insert(*nodeUUID82Ptr, *nodeUUID81Ptr, TrustLineDirection::Both);
    mStorageHandler->routingTablesHandler()->routingTable2Level()->rollBack();
    mStorageHandler->routingTablesHandler()->routingTable2Level()->commit();
    mStorageHandler->routingTablesHandler()->routingTable2Level()->insert(*nodeUUID81Ptr, *nodeUUID83Ptr, TrustLineDirection::Incoming);
    mStorageHandler->routingTablesHandler()->routingTable2Level()->insert(*nodeUUID81Ptr, *nodeUUID84Ptr, TrustLineDirection::Both);
    mStorageHandler->routingTablesHandler()->routingTable2Level()->insert(*nodeUUID83Ptr, *nodeUUID81Ptr, TrustLineDirection::Outgoing);


    vector<tuple<NodeUUID, NodeUUID, TrustLineDirection>> records = mStorageHandler->routingTablesHandler()->routingTable2Level()->routeRecordsWithDirections();
    NodeUUID source;
    NodeUUID target;
    TrustLineDirection direction;
    for (auto &record : records) {
        std::tie(source, target, direction) = record;
        info() << source << " " << target << " " << direction;
    }

    for (auto const &itMap : mStorageHandler->routingTablesHandler()->routingTable2Level()->routeRecordsWithDirectionsMapSourceKey()) {
        info() << "key: " << itMap.first;
        for (auto const &itVector : itMap.second) {
            info() << "\tvalue: " << itVector.first << " " << itVector.second;
        }
    }
    delete nodeUUID81Ptr;
    delete nodeUUID82Ptr;
    delete nodeUUID83Ptr;
    delete nodeUUID84Ptr;
    delete nodeUUID85Ptr;
    delete nodeUUID86Ptr;

}

void PathsManager::testTrustLineHandler() {

    for (auto const &trustLine : mTrustLinesManager->trustLines()) {
        mStorageHandler->trustLineHandler()->insert(trustLine.second);
    }
    TrustLine::Shared trLine = make_shared<TrustLine>(
        mNodeUUID,
        TrustLineAmount(100),
        TrustLineAmount(200),
        TrustLineBalance(-30));
    mStorageHandler->trustLineHandler()->insert(trLine);
    mStorageHandler->trustLineHandler()->commit();
    info() << mNodeUUID << "is present: " << mStorageHandler->trustLineHandler()->containsContractor(mNodeUUID);
    mStorageHandler->trustLineHandler()->trustLines();
    trLine->setBalance(55);
    trLine->setIncomingTrustAmount(1000);
    mStorageHandler->trustLineHandler()->update(trLine);
    mStorageHandler->trustLineHandler()->trustLines();
    mStorageHandler->trustLineHandler()->deleteTrustLine(mNodeUUID);
    info() << mNodeUUID << "is present: " << mStorageHandler->trustLineHandler()->containsContractor(mNodeUUID);
    mStorageHandler->trustLineHandler()->trustLines();
}

void PathsManager::fillBigRoutingTables() {

    uint32_t firstLevelNode = 20;
    uint32_t countNodes = 1000;
    srand (time(NULL));
    vector<NodeUUID*> nodeUUIDPtrs;
    nodeUUIDPtrs.reserve(countNodes);
    for (uint32_t idx = 1; idx <= countNodes; idx++) {
        nodeUUIDPtrs.push_back(new NodeUUID(nodeUUIDName(idx)));
    }

    uint32_t currentIdx = 0;
    if (!mNodeUUID.stringUUID().compare("13e5cf8c-5834-4e52-b65b-000000000001")) {
        currentIdx = 1;
    }

    if (!mNodeUUID.stringUUID().compare("13e5cf8c-5834-4e52-b65b-000000000002")) {
        currentIdx = 2;
    }

    vector<uint32_t> firstLevelNodes;
    while (firstLevelNodes.size() < firstLevelNode) {
        uint32_t  nextIdx = rand() % countNodes + 1;
        if(std::find(firstLevelNodes.begin(), firstLevelNodes.end(), nextIdx) != firstLevelNodes.end() ||
           nextIdx == currentIdx) {
            continue;
        }
        firstLevelNodes.push_back(nextIdx);
    }
    info() << "PathsManager::fillBigRoutingTables first level done";
    set<uint32_t> secondLevelAllNodes;
    for (auto firstLevelIdx : firstLevelNodes) {
        vector<uint32_t> secondLevelNodes;
        while (secondLevelNodes.size() < firstLevelNode) {
            uint32_t  nextIdx = rand() % countNodes + 1;
            if(std::find(secondLevelNodes.begin(), secondLevelNodes.end(), nextIdx) != secondLevelNodes.end() ||
               nextIdx == firstLevelIdx) {
                continue;
            }
            secondLevelNodes.push_back(nextIdx);
            secondLevelAllNodes.insert(nextIdx);
            mStorageHandler->routingTablesHandler()->routingTable2Level()->insert(
                *getPtrByNodeNumber(
                    firstLevelIdx,
                    nodeUUIDPtrs),
                *getPtrByNodeNumber(
                    nextIdx,
                    nodeUUIDPtrs),
                TrustLineDirection::Both);
        }
        mStorageHandler->routingTablesHandler()->routingTable2Level()->commit();
        info() << "fillBigRoutingTables:: second level commit";
    }
    info() << "fillBigRoutingTables:: second level done";
    for (auto secondLevelIdx : secondLevelAllNodes) {
        vector<uint32_t> thirdLevelNodes;
        while (thirdLevelNodes.size() < firstLevelNode) {
            uint32_t  nextIdx = rand() % countNodes + 1;
            if(std::find(thirdLevelNodes.begin(), thirdLevelNodes.end(), nextIdx) != thirdLevelNodes.end() ||
               nextIdx == secondLevelIdx) {
                continue;
            }
            thirdLevelNodes.push_back(nextIdx);
            mStorageHandler->routingTablesHandler()->routingTable3Level()->insert(
                *getPtrByNodeNumber(
                    secondLevelIdx,
                    nodeUUIDPtrs),
                *getPtrByNodeNumber(
                    nextIdx,
                    nodeUUIDPtrs),
                TrustLineDirection::Both);
        }
        mStorageHandler->routingTablesHandler()->routingTable3Level()->commit();
        info() << "fillBigRoutingTables:: third level commit";
    }
    info() << "fillBigRoutingTables:: third level done";
    for (auto nodeUUIDPrt : nodeUUIDPtrs) {
        delete nodeUUIDPrt;
    }
}

string PathsManager::nodeUUIDName(uint32_t number) {
    stringstream s;
    s << number;
    string numStr = s.str();
    while (numStr.length() < 12) {
        numStr = "0" + numStr;
    }
    return "13e5cf8c-5834-4e52-b65b-" + numStr;
}

NodeUUID* PathsManager::getPtrByNodeNumber(
    uint32_t number,
    vector<NodeUUID*> nodeUUIDPtrs) {

    string nodeUUIDStr = nodeUUIDName(number);
    for (auto nodeUUIDPtr : nodeUUIDPtrs) {
        if (nodeUUIDStr.compare(nodeUUIDPtr->stringUUID()) == 0) {
            return nodeUUIDPtr;
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
