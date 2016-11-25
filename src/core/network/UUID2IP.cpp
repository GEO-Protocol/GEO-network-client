#include "UUID2IP.h"

UUID2IP::UUID2IP() {
    boost::uuids::uuid contractor = boost::uuids::random_generator()();
    mCache.insert(pair<uuids::uuid, pair<string, uint16_t>>(contractor, make_pair(string("127.0.0.1"), 8086)));
}

UUID2IP::~UUID2IP() {
    mCache.clear();
}

void UUID2IP::registerInGlobalCache() {}

const pair<string, uint16_t> &UUID2IP::getNodeAddress(const uuids::uuid &contractorUuid) const {
    if (isNodeAddressExistInLocalCache(contractorUuid)) {
        return mCache.at(contractorUuid);
    }
    //What value must be returned in case if uuid in cache not founded
    return make_pair("", 0);
}

const bool UUID2IP::isNodeAddressExistInLocalCache(const uuids::uuid &nodeUuid) const{
    return mCache.count(nodeUuid) > 0;
}

void UUID2IP::compressLocalCache() {}

