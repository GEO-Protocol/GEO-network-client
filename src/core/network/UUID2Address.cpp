#include "UUID2Address.h"

UUID2Address::UUID2Address(
    as::io_service &IOService,
    const string host,
    const uint16_t port):

    mIOService(IOService),
    mResolver(mIOService),
    mServiceIP(host),
    mServicePort(port),
    mQuery(mServiceIP, boost::lexical_cast<string>(mServicePort)),
    mSocket(mIOService),
    mRequestStream(&mRequest) {

#ifdef INTERNAL_ARGUMENTS_VALIDATION
    assert(logger != nullptr);
#endif

    mEndpointIterator = mResolver.resolve(mQuery);
}

UUID2Address::~UUID2Address() {
    mRequest.consume(mRequest.size());
    mSocket.close();
}

void UUID2Address::registerInGlobalCache(
    const NodeUUID &uuid,
    const string &nodeHost,
    const uint16_t &nodePort) {

    stringstream url;
    url << "/api/v1/nodes/"
        << boost::lexical_cast<string>(uuid)
        << "/";

    stringstream host;
    host << mServiceIP << ":" << mServicePort;

    stringstream body;
    body << "{"
         << "\"ip_address\": \"" << nodeHost << "\","
         << "\"port\": \"" << nodePort << "\""
         << "}";


    mRequest.consume(mRequest.size());
    mRequestStream << "POST " << url.str() << " HTTP/1.0\r\n";
    mRequestStream << "Host: " << host.str() << "\r\n";
    mRequestStream << "Accept: */*\r\n";
    mRequestStream << "Content-Length: " << body.str().length() << "\r\n";
    mRequestStream << "Content-Type: application/json\r\n";
    mRequestStream << "Connection: close\r\n\r\n";
    mRequestStream << body.str();

    as::connect(mSocket, mEndpointIterator);
    as::write(mSocket, mRequest);

    json result = json::parse(processResponse());
    int code = result.value("code", -1);
    if (code != 0) {
        throw Exception(
            "UUID2Address::registerInGlobalCache: "
                "Cant issue the request.");
    }
}

const pair<string, uint16_t> UUID2Address::fetchFromGlobalCache(
    const NodeUUID &uuid) {

    std::stringstream url;
    url << "/api/v1/nodes/"
        << boost::lexical_cast<string>(uuid.stringUUID())
        << "/";

    std::stringstream host;
    host << mServiceIP << ":" << mServicePort;

    mRequest.consume(mRequest.size());
    mRequestStream << "GET " << url.str() << " HTTP/1.0\r\n";
    mRequestStream << "Host: " << host.str() << "\r\n";
    mRequestStream << "Accept: */*\r\n";
    mRequestStream << "Connection: close\r\n\r\n";

    boost::asio::connect(mSocket, mEndpointIterator);
    boost::asio::write(mSocket, mRequest);

    json result = json::parse(processResponse());
    int code = result.value("code", -1);
    if (code == 0) {
        json data = result["data"];
        string ip = data.value("ip_address", "");
        int port = data.value("port", -1);
        if (!ip.compare("") && port > -1) {
            pair <string, uint16_t> remoteNodeAddressPair = make_pair(ip, (uint16_t) port);
            mCache.insert(pair <uuids::uuid, pair<string, uint16_t>> (uuid, remoteNodeAddressPair));
            mLastAccessTime.insert(pair<uuids::uuid, long>(uuid, getCurrentTimestamp()));
            return remoteNodeAddressPair;
        } else {
            throw ConflictError("Incorrect ip address value from remote service.");
        }
    } else {
        throw ConflictError("Illegal result code from remote service.");
    }
}

const string UUID2Address::processResponse() {
    boost::asio::streambuf response;
    std::istream responseStream(&response);

    boost::asio::read_until(mSocket, response, "\r\n");

    std::string httpVersion;
    responseStream >> httpVersion;
    unsigned int statusCode;
    responseStream >> statusCode;
    std::string statusMessage;
    getline(responseStream, statusMessage);
    if (!responseStream || httpVersion.substr(0, 5) != "HTTP/") {
        throw ValueError(
            "UUID2Address::processResponse: "
                "Invalid response from the remote service");
    }

    if (statusCode == 200) {
        boost::asio::read_until(mSocket, response, "\r\n\r\n");

        string header;
        while (std::getline(responseStream, header) && header != "\r") {}
        if (response.size() > 0) {
            std::string data((std::istreambuf_iterator<char>(&response)), std::istreambuf_iterator<char>());
            return data;
        } else {
            throw ValueError("Empty response body from the remote service");
        }
    } else {
        throw IOError("Bad request to the remote service");
    }
}

const pair <string, uint16_t> UUID2Address::getNodeAddress(const uuids::uuid &contractorUUID) {
    if (isNodeAddressExistInLocalCache(contractorUUID)) {
        if (wasAddressAlreadyCached(contractorUUID)) {
            map<uuids::uuid, long>::iterator it = mLastAccessTime.find(contractorUUID);
            if (it != mLastAccessTime.end()) {
                it->second = getCurrentTimestamp();
            }
        } else {
            mLastAccessTime.insert(pair<uuids::uuid, long>(contractorUUID, getCurrentTimestamp()));
        }
        return mCache.at(contractorUUID);
    } else {
        return fetchFromGlobalCache(contractorUUID);
    }
}

const bool UUID2Address::isNodeAddressExistInLocalCache(const uuids::uuid &nodeUUID) {
    return mCache.count(nodeUUID) > 0;
}

void UUID2Address::compressLocalCache() {
    map<uuids::uuid, long>::iterator it;
    for (it = mLastAccessTime.begin(); it != mLastAccessTime.end(); ++it){
        long timeOfLastUsing = it->second;
        if (timeOfLastUsing < getYesterdayTimestamp()){
            mCache.erase(it->first);
            mLastAccessTime.erase(it->first);
        }
    }
}

const long UUID2Address::getCurrentTimestamp() {
    return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

const long UUID2Address::getYesterdayTimestamp() {
    std::chrono::time_point<std::chrono::system_clock> yesterday = std::chrono::system_clock::now() - std::chrono::hours(24);
    return std::chrono::duration_cast<std::chrono::seconds>(yesterday.time_since_epoch()).count();
}

const bool UUID2Address::wasAddressAlreadyCached(const uuids::uuid &nodeUUID) {
    return mLastAccessTime.count(nodeUUID) > 0;
}



