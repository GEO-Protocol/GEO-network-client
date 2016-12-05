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
#include <boost/lexical_cast.hpp>
#include "../../libs/json.h"
#include "../logger/Logger.h"
#include "../common/exceptions/ValueError.h"
#include "../common/exceptions/IOError.h"
#include "../common/exceptions/ConflictError.h"

using namespace std;
using boost::asio::ip::tcp;
using boost::asio::ip::address;
using json = nlohmann::json;

namespace uuids = boost::uuids;

class UUID2IP {

private:
    map <uuids::uuid, pair<string, uint16_t >> mCache;

    string mServiceIP;
    string mServicePort;
    boost::asio::io_service mIOService;
    tcp::resolver mResolver;
    tcp::resolver::query mQuery;
    tcp::resolver::iterator mEndpointIterator;
    tcp::socket mSocket;
    boost::asio::streambuf mRequest;
    std::ostream mRequestStream;

    Logger mLogger;
    static const constexpr char* kSubsystemName = "UUID2IP client";

public:
    UUID2IP(string serviceIP, string servicePort);

    ~UUID2IP();

private:
    void registerInGlobalCache(const uuids::uuid &nodeUUID, const string &nodeIP, const string &nodePort);

    const pair<string, uint16_t> fetchFromGlobalCache(const uuids::uuid &nodeUUID);

    const string processResponse();

    const pair<string, uint16_t> getNodeAddress(const uuids::uuid &contractorUuid);

    const bool isNodeAddressExistInLocalCache(const uuids::uuid &nodeUuid);

    void compressLocalCache();

};

#endif //GEO_NETWORK_CLIENT_UUID2IP_H
