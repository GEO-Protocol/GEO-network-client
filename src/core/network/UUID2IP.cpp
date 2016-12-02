#include "UUID2IP.h"

UUID2IP::UUID2IP(string serviceIP, string servicePort) :
        mServiceIP(serviceIP), mServicePort(servicePort), mResolver(mIOService),
        mQuery(mServiceIP, mServicePort), mSocket(mIOService), mRequestStream(&mRequest) {
    mEndpointIterator = mResolver.resolve(mQuery);
    boost::asio::connect(mSocket, mEndpointIterator);
}

UUID2IP::~UUID2IP() {
    mCache.clear();
    socket.close();
}

void UUID2IP::registerInGlobalCache(uuids::uuid nodeUUID, string nodeIP, string nodePort) {
    string requestBody = "{\"ip_address\"" + ":" + "\"" + nodeIP + "\"," +
                         "\"port\"" + ":" + "\"" + nodePort + "\"}";

    string uuid = boost::lexical_cast<string>(nodeUUID);
    string urlParams = "/api/v1/nodes/" + uuid + "/";
    string host = mServiceIP + ":" + mServicePort;
    requestStream << "POST " << urlParams << " HTTP/1.0\r\n";
    requestStream << "Host: " << host << "\r\n";
    requestStream << "Accept: */*\r\n";
    requestStream << "Content-Length: " << requestBody.length() << "\r\n";
    requestStream << "Content-Type: application/json\r\n";
    requestStream << "Connection: close\r\n\r\n";
    requestStream << requestBody;

    boost::asio::write(mSocket, mRequest);

    json result = json::parse(processResponse());
    int code = result.value("code", -1);
    if (code == 0){
        json data = json::parse(result.value("data"));
        string ip = data.value("ip_address");
        int port = data.value("port");
        cout << ip << " " << port;
    }
}

void UUID2IP::fetchFromGlobalCache(uuids::uuid nodeUUID) {
    string uuid = boost::lexical_cast<string>(nodeUUID);
    string urlParams = "/api/v1/nodes/" + uuid + "/";
    string host = mServiceIP + ":" + mServicePort;
    requestStream << "GET " << urlParams << " HTTP/1.0\r\n";
    requestStream << "Host: " << host << "\r\n";
    requestStream << "Accept: */*\r\n";
    requestStream << "Connection: close\r\n\r\n";

    boost::asio::write(mSocket, mRequest);

    json result = json::parse(processResponse());
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
    responseStream >> statusMessage;
    if (!responseStream || httpVersion.substr(0, 5) != "HTTP/") {
        throw ValueError("Invalid response from the remote service");
    }

    if (statusCode == 200) {
        boost::asio::read_until(socket, response, "\r\n\r\n");

        string header;
        while (std::getline(responseStream, header) && header != "\r") {
            cout << header << endl;
            cout << endl;
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

const pair <string, uint16_t> &UUID2IP::getNodeAddress(const uuids::uuid &contractorUuid) const {
    if (isNodeAddressExistInLocalCache(contractorUuid)) {
        return mCache.at(contractorUuid);
    }
    return make_pair("", 0);
}

const bool UUID2IP::isNodeAddressExistInLocalCache(const uuids::uuid &nodeUuid) const {
    return mCache.count(nodeUuid) > 0;
}

void UUID2IP::compressLocalCache() {}



