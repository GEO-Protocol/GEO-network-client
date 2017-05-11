#ifndef GEO_NETWORK_CLIENT_UUID2IP_H
#define GEO_NETWORK_CLIENT_UUID2IP_H

#include "../common/Types.h"

#include "../../../../common/NodeUUID.h"
#include "../../../../common/exceptions/ValueError.h"
#include "../../../../common/exceptions/IOError.h"
#include "../../../../common/exceptions/ConflictError.h"
#include "../../../../logger/Logger.h"

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>

#include "../../../../../libs/json/json.h"

#include <string>
#include <iostream>
#include <sstream>
#include <chrono>
#include <map>


namespace uuids = boost::uuids;
namespace as = boost::asio;

using namespace std;
using as::ip::tcp;
using as::ip::address;
using json = nlohmann::json;


class UUID2Address {
public:
    UUID2Address(
        as::io_service &IOService,
        const string host,
        const uint16_t port = 80);

    ~UUID2Address();

    void registerInGlobalCache(
        const NodeUUID &uuid,
        const string &nodeHost,
        const uint16_t &nodePort);

    UDPEndpoint& endpoint (
        const NodeUUID &contractorUUUID);

private:
    UDPEndpoint& fetchFromGlobalCache(
        const NodeUUID &uuid);

    const pair<unsigned int, string> processResponse();

    // ToDo: implement me back
//    void compressLocalCache();

private:
    map<NodeUUID, UDPEndpoint> mCache;
    map<NodeUUID, TimePoint> mLastAccessTime; // todo: specify this on boost::chrono types

    string mServiceIP;
    uint16_t mServicePort;

    boost::asio::io_service &mIOService;

    tcp::socket mSocket;
    tcp::resolver mResolver;
    tcp::resolver::query mQuery;
    tcp::resolver::iterator mEndpointIterator;
    boost::asio::streambuf mRequest;
    std::ostream mRequestStream;
};

#endif //GEO_NETWORK_CLIENT_UUID2IP_H
