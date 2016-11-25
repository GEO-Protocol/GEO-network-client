#ifndef GEO_NETWORK_CLIENT_UUID2IP_H
#define GEO_NETWORK_CLIENT_UUID2IP_H

#include <map>
#include <string>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include "Communicator.h"

using namespace std;

namespace uuids = boost::uuids;

class Communicator;

class UUID2IP {
    friend class Communicator;

private:
    map <uuids::uuid, pair<string, uint16_t >> mCache;

private:
    UUID2IP();

    ~UUID2IP();

    void registerInGlobalCache();

    const pair<string, uint16_t> &getNodeAddress(const uuids::uuid &contractorUuid) const;

    const bool isNodeAddressExistInLocalCache(const uuids::uuid &nodeUuid) const;

    void compressLocalCache();

};

#endif //GEO_NETWORK_CLIENT_UUID2IP_H
