#include "TrustLinesStorage.h"

TrustLinesStorage::TrustLinesStorage(
        const string &fileName) :

        UUIDMapBlockStorage(
                "io/trust_lines",
                fileName) {}

const vector<NodeUUID> TrustLinesStorage::getAllContractorsUUIDs() {

    const vector<storage::uuids::uuid> *uuidKeys = keys();
    vector<NodeUUID> nodesKeys;
    for (auto &it : *uuidKeys) {
        nodesKeys.push_back(NodeUUID(it));
    }
    delete uuidKeys;
    return nodesKeys;
}