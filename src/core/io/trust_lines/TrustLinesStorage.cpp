#include "TrustLinesStorage.h"

TrustLinesStorage::TrustLinesStorage(const string &fileName) : UUIDMapBlockStorage(fileName) {}

TrustLinesStorage::~TrustLinesStorage() {}

const vector<NodeUUID> TrustLinesStorage::getAllContractorsUUIDs() {
    return keys();
}

void TrustLinesStorage::writeNewTrustLineInStorage(const NodeUUID &uuid, const byte *data, const size_t bytesCount) {
    write(uuid, data, bytesCount);
}

void TrustLinesStorage::modifyExistingTrustLineInStorage(const NodeUUID &uuid, const byte *data, const size_t bytesCount) {
    try{
        rewrite(uuid, data, bytesCount);
    }catch (std::exception &e){
        throw IOError(string(string("Can't write trust line in storage. Details: ") + string(e.what())).c_str());
    }
}

const io::Block *TrustLinesStorage::readTrustLineFromStorage(const NodeUUID &uuid) {
    io::Block *block;
    try{
        block = readFromFile(uuid);
    }catch (std::exception &e){
        throw IOError(string(string("Can't read trust line data from storage. Details: ") + string(e.what())).c_str());
    }
    return block;
}

void TrustLinesStorage::removeTrustLineFromStorage(const NodeUUID &uuid) {
    try{
        erase(uuid);
    }catch (std::exception &e){
        throw IOError(string(string("Can't remove trust line from storage. Details: ") + string(e.what())).c_str());
    }
}