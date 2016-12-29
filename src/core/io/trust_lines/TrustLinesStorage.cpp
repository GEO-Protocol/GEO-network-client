#include "TrustLinesStorage.h"

TrustLinesStorage::TrustLinesStorage(const string &fileName) : UUIDMapBlockStorage("storage", fileName) {}

TrustLinesStorage::~TrustLinesStorage() {}

const vector<NodeUUID> TrustLinesStorage::getAllContractorsUUIDs() {
    vector<io::uuids::uuid> uuidKeys = keys();
    vector<NodeUUID> nodesKeys;
    for (auto &it : uuidKeys) {
        nodesKeys.push_back(NodeUUID(it));
    }
    return nodesKeys;
}

void TrustLinesStorage::writeNewTrustLineInStorage(const NodeUUID &uuid, const byte *data, const size_t bytesCount) {
    //Create boost::uuids::uuid from NodeUUID, 'cause UUIDMapBlockStorage use type of boost::uuids::uuid
    write(io::uuids::uuid(uuid), data, bytesCount);
}

void TrustLinesStorage::modifyExistingTrustLineInStorage(const NodeUUID &uuid, const byte *data, const size_t bytesCount) {
    try{
        rewrite(io::uuids::uuid(uuid), data, bytesCount);
    }catch (std::exception &e){
        throw IOError(string(string("Can't write trust line in storage. Details: ") + string(e.what())).c_str());
    }
}

const io::Block *TrustLinesStorage::readTrustLineFromStorage(const NodeUUID &uuid) {
    io::Block *block;
    try{
        block = readFromFile(io::uuids::uuid(uuid));
    }catch (std::exception &e){
        throw IOError(string(string("Can't read trust line data from storage. Details: ") + string(e.what())).c_str());
    }
    return block;
}

void TrustLinesStorage::removeTrustLineFromStorage(const NodeUUID &uuid) {
    try{
        erase(io::uuids::uuid(uuid));
    }catch (std::exception &e){
        throw IOError(string(string("Can't remove trust line from storage. Details: ") + string(e.what())).c_str());
    }
}