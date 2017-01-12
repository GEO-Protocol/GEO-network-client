#include "UUID2Address.h"
#include "../common/exceptions/NotFoundError.h"

UUID2Address::UUID2Address(
    as::io_service &IOService,
    const string host,
    const uint16_t port) :

    mIOService(IOService),
    mResolver(mIOService),
    mServiceIP(host),
    mServicePort(port),
    mQuery(mServiceIP, boost::lexical_cast<string>(mServicePort)),
    mSocket(mIOService),
    mRequestStream(&mRequest) {

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

    auto response = processResponse();
    if (response.first != 200) {
        throw Exception("UUID2Address::registerInGlobalCache: "
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

    auto result = processResponse();
    switch (result.first) {
        case 200: {
            json jsonResponse = json::parse(result.second);
            json data = jsonResponse["data"];
            string ip = data.value("ip_address", "");
            int port = data.value("port", -1);
            if (port > -1) {
                pair<string, uint16_t> remoteNodeAddressPair = make_pair(ip, (uint16_t) port);
                mCache.insert(
                    make_pair(
                        uuid, remoteNodeAddressPair
                    )
                );
                mLastAccessTime.insert(
                    make_pair(
                        uuid,
                        currentTimestamp()
                    )
                );
                return remoteNodeAddressPair;
            } else {
                throw ConflictError("UUID2Address::fetchFromGlobalCache: "
                                        "Incorrect ip address value from remote service.");
            }
        }

        case 400: {
            throw NotFoundError("UUID2Address::fetchFromGlobalCache: "
                                    "One or sevaral request parameters are invalid.");
        }

        case 404: {
            throw NotFoundError("UUID2Address::fetchFromGlobalCache: "
                                    "There is no node with exact UUID.");
        }

        default: {
            throw ConflictError("UUID2Address::fetchFromGlobalCache: "
                                    "Illegal result code from remote service.");
        }
    }
}

const pair<unsigned int, string> UUID2Address::processResponse() {

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

    switch (statusCode) {
        case 200: {
            boost::asio::read_until(mSocket, response, "\r\n\r\n");
            string header;
            while (getline(responseStream, header) && header != "\r") {}
            if (response.size() > 0) {
                string data(
                    (istreambuf_iterator<char>(&response)),
                    istreambuf_iterator<char>()
                );
                return make_pair(
                    statusCode,
                    data
                );

            } else {
                throw ValueError("UUID2Address::processResponse: "
                                     "Empty response body from the remote service");
            }
        }

        case 400:
        case 404:
        case 500: {
            return make_pair(
                statusCode,
                string()
            );
        }

        default: {
            throw IOError("UUID2Address::processResponse: "
                              "Unexpected response code");
        }
    }
}

const pair<string, uint16_t> UUID2Address::getNodeAddress(
    const uuids::uuid &contractorUUID) {

    if (isNodeAddressExistInLocalCache(contractorUUID)) {
        if (wasAddressAlreadyCached(contractorUUID)) {
            auto it = mLastAccessTime.find(contractorUUID);
            if (it != mLastAccessTime.end()) {
                it->second = currentTimestamp();
            }

        } else {
            mLastAccessTime.insert(make_pair(contractorUUID, currentTimestamp()));
        }
        return mCache.at(contractorUUID);

    } else {
        return fetchFromGlobalCache(contractorUUID);
    }
}

const bool UUID2Address::isNodeAddressExistInLocalCache(
    const uuids::uuid &nodeUUID) {

    return mCache.count(nodeUUID) > 0;
}

void UUID2Address::compressLocalCache() {

    for (auto const &it : mLastAccessTime) {
        long timeOfLastUsing = it.second;
        if (timeOfLastUsing < yesterdayTimestamp()) {
            mCache.erase(it.first);
            mLastAccessTime.erase(it.first);
        }
    }
}

const long UUID2Address::currentTimestamp() {

    return std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
}

const long UUID2Address::yesterdayTimestamp() {

    std::chrono::time_point<std::chrono::system_clock> yesterday = std::chrono::system_clock::now() - std::chrono::hours(24);
    return std::chrono::duration_cast<std::chrono::seconds>(
        yesterday.time_since_epoch()
    ).count();
}

const bool UUID2Address::wasAddressAlreadyCached(
    const uuids::uuid &nodeUUID) {

    return mLastAccessTime.count(nodeUUID) > 0;
}



