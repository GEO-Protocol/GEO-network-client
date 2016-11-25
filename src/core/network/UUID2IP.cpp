#include "UUID2IP.h"

UUID2IP::UUID2IP() {
}

UUID2IP::~UUID2IP() {}

void UUID2IP::registerInGlobalCache() {}

const pair<string, uint16_t> &UUID2IP::getNodeIpAddress(const uuids::uuid &contractorUuid) const {
    if (isNodeAddressExistInLocalCache(contractorUuid)) {
        return mCache.at(contractorUuid);
    }
}

bool UUID2IP::isNodeAddressExistInLocalCache(const uuids::uuid &nodeUuid) {
    return mCache.count(nodeUuid) > 0;
}

void UUID2IP::compressLocalCache() {}

