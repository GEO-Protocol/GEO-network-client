#include "UUID2IP.h"

UUID2IP::UUID2IP(string serviceIP, string servicePort) :
        mServiceIP(serviceIP), mServicePort(servicePort), mResolver(mIOService),
        mQuery(mServiceIP, mServicePort), mSocket(mIOService), mRequestStream(&mRequest) {
    mEndpointIterator = mResolver.resolve(mQuery);

    /*uuids::uuid randomUUID = uuids::random_generator()();
    registerInGlobalCache(randomUUID, "195.150.1.1", "7777");*/
}

UUID2IP::~UUID2IP() {
    mCache.clear();
    mRequest.consume(mRequest.size());
    mSocket.close();
}

void UUID2IP::registerInGlobalCache(const uuids::uuid &nodeUUID, const string &nodeIP, const string &nodePort) {
    string requestBody = string("{\"ip_address\"") + string(":") + string("\"") + nodeIP + string("\",") +
                         string("\"port\"") + string(":") + string("\"") + nodePort + string("\"}");

    string uuid = boost::lexical_cast<string>(nodeUUID);
    string urlParams = "/api/v1/nodes/" + uuid + "/";
    string host = mServiceIP + ":" + mServicePort;

    mRequest.consume(mRequest.size());
    mRequestStream << "POST " << urlParams << " HTTP/1.0\r\n";
    mRequestStream << "Host: " << host << "\r\n";
    mRequestStream << "Accept: */*\r\n";
    mRequestStream << "Content-Length: " << requestBody.length() << "\r\n";
    mRequestStream << "Content-Type: application/json\r\n";
    mRequestStream << "Connection: close\r\n\r\n";
    mRequestStream << requestBody;

    boost::asio::connect(mSocket, mEndpointIterator);
    boost::asio::write(mSocket, mRequest);

    json result = json::parse(processResponse());
    int code = result.value("code", -1);
    if (code == 0) {
        mLogger.logInfo(kSubsystemName, string("Registered in UUID2IP system global cache"));
    }
}

const pair <string, uint16_t> UUID2IP::fetchFromGlobalCache(const uuids::uuid &nodeUUID) {
    string uuid = boost::lexical_cast<string>(nodeUUID);
    string urlParams = "/api/v1/nodes/" + uuid + "/";
    string host = mServiceIP + ":" + mServicePort;

    mRequest.consume(mRequest.size());
    mRequestStream << "GET " << urlParams << " HTTP/1.0\r\n";
    mRequestStream << "Host: " << host << "\r\n";
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
            mCache.insert(pair <uuids::uuid, pair<string, uint16_t>> (nodeUUID, remoteNodeAddressPair));
            mLastAccessTime.insert(pair<uuid::uuid, long>(nodeUUID, getCurrentTimestamp()));
            return remoteNodeAddressPair;
        } else {
            throw ConflictError("Incorrect ip address value from remote service.");
        }
    } else {
        throw ConflictError("Illegal result code from remote service.");
    }
}

const string UUID2IP::processResponse() {
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
        throw ValueError("Invalid response from the remote service");
    }

    if (statusCode == 200) {
        boost::asio::read_until(mSocket, response, "\r\n\r\n");

        string header;
        while (std::getline(responseStream, header) && header != "\r") {
            mLogger.logInfo(kSubsystemName, header);
        }
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

const pair <string, uint16_t> UUID2IP::getNodeAddress(const uuids::uuid &contractorUUID) {
    if (isNodeAddressExistInLocalCache(contractorUuid)) {
        if (wasAddressAlreadyCached(contractorUUID)) {
            map<uuid::uuid, long>::iterator it = mLastAccessTime.find(contractorUUID);
            if (it != mLastAccessTime.end()) {
                it->second = getCurrentTimestamp();
            }
        } else {
            mLastAccessTime.insert(pair<uuid::uuid, long>(contractorUUID, getCurrentTimestamp()));
        }
        return mCache.at(contractorUuid);
    } else {
        return fetchFromGlobalCache(contractorUuid);
    }
}

const bool UUID2IP::isNodeAddressExistInLocalCache(const uuids::uuid &nodeUUID) {
    return mCache.count(nodeUuid) > 0;
}

void UUID2IP::compressLocalCache() {
    map<uuids::uuid, long>::iteratot iterator;
    for (iterator it = mLastAccessTime.begin(); it != mLastAccessTime.end(); ++it){
        long timeOfLastUsing = it->second;
        if (timeOfLastUsing < getYesterdayTimestamp()){
            mCache.erase(it->first);
            mLastAccessTime.erase(it->first);
        }
    }
}

const long UUID2IP::getCurrentTimestamp() {
    return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

const long UUID2IP::getYesterdayTimestamp() {
    std::chrono::time_point<std::chrono::system_clock> yesterday = std::chrono::system_clock::now() - std::chrono::hours(24);
    return std::chrono::duration_cast<std::chrono::seconds>(yesterday.time_since_epoch()).count();
}

const bool UUID2IP::wasAddressAlreadyCached(const uuids::uuid &nodeUUID) {
    return mLastAccessTime.count(nodeUUID) > 0;
}



