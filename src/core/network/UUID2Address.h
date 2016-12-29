#ifndef GEO_NETWORK_CLIENT_UUID2IP_H
#define GEO_NETWORK_CLIENT_UUID2IP_H

#include "../common/NodeUUID.h"

#include <string>
#include <iostream>
#include <sstream>
#include <chrono>
#include <map>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>

#include "../../libs/json/json.h"

#include "../logger/Logger.h"
#include "../common/exceptions/ValueError.h"
#include "../common/exceptions/IOError.h"
#include "../common/exceptions/ConflictError.h"


namespace uuids = boost::uuids;
namespace as = boost::asio;

using namespace std;
using boost::asio::ip::tcp;
using boost::asio::ip::address;
using json = nlohmann::json;


class UUID2Address {
public:
    UUID2Address(
        as::io_service &IOService,
        const string host,
        const uint16_t port=80);

    ~UUID2Address();

    void registerInGlobalCache(
        const NodeUUID &uuid,
        const string &nodeHost,
        const uint16_t &nodePort);

protected:
    map<uuids::uuid, pair<string, uint16_t>> mCache;
    map<uuids::uuid, long> mLastAccessTime; // todo: specify this on boost::chrono types

    string mServiceIP;
    uint16_t mServicePort;

    boost::asio::io_service &mIOService;

    tcp::socket mSocket;
    tcp::resolver mResolver;
    tcp::resolver::query mQuery;
    tcp::resolver::iterator mEndpointIterator;
    boost::asio::streambuf mRequest;
    std::ostream mRequestStream;


private:
    const pair<string, uint16_t> fetchFromGlobalCache(
        const NodeUUID &uuid);

    const string processResponse();

    const pair<string, uint16_t> getNodeAddress(
        const uuids::uuid &contractorUUUID);

    const bool isNodeAddressExistInLocalCache(
        const uuids::uuid &nodeUUID);

    void compressLocalCache();

    const long getCurrentTimestamp(); // todo: specify this on boost::chrono types
    const long getYesterdayTimestamp(); // todo: specify this on boost::chrono types
    const bool wasAddressAlreadyCached(const uuids::uuid &nodeUUID);

};

#endif //GEO_NETWORK_CLIENT_UUID2IP_H
