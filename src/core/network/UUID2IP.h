#ifndef GEO_NETWORK_CLIENT_UUID2IP_H
#define GEO_NETWORK_CLIENT_UUID2IP_H

#include <map>
#include <string>
#include <iostream>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include "Communicator.h"

using namespace std;
using boost::asio::ip::tcp;

namespace uuids = boost::uuids;

class UUID2IP {

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
