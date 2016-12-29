#include "TrustLinesStorage.h"

TrustLinesStorage::TrustLinesStorage(
        const string &fileName) :

        UUIDMapBlockStorage(
                "storage",
                fileName) {}

TrustLinesStorage::~TrustLinesStorage() {}

const vector<NodeUUID> TrustLinesStorage::getAllContractorsUUIDs() {

    vector<storage::uuids::uuid> uuidKeys = keys();
    vector<NodeUUID> nodesKeys;
    for (auto &it : uuidKeys) {
        nodesKeys.push_back(NodeUUID(it));
    }
    return nodesKeys;
}