#include "UUID2IP.h"

UUID2IP::UUID2IP(string serviceIP, string servicePort) :
        mServiceIP(serviceIP), mServicePort(servicePort), mResolver(mIOService),
        mQuery(mServiceIP, mServicePort), mSocket(mIOService), mRequestStream(&mRequest) {
    mEndpointIterator = mResolver.resolve(mQuery);
    boost::asio::connect(mSocket, mEndpointIterator);
    uuids::uuid randomUUID = uuids::random_generator()();
    //registerInGlobalCache(randomUUID, "195.150.1.1", "7777");
    fetchFromGlobalCache(randomUUID);
}

UUID2IP::~UUID2IP() {
    mCache.clear();
    mSocket.close();
}

void UUID2IP::registerInGlobalCache(uuids::uuid nodeUUID, string nodeIP, string nodePort) {
    string requestBody = string("{\"ip_address\"") + string(":") + string("\"") + nodeIP + string("\",") +
                         string("\"port\"") + string(":") + string("\"") + nodePort + string("\"}");

    string uuid = boost::lexical_cast<string>(nodeUUID);
    string urlParams = "/api/v1/nodes/" + uuid + "/";
    string host = mServiceIP + ":" + mServicePort;
    mRequestStream << "POST " << urlParams << " HTTP/1.0\r\n";
    mRequestStream << "Host: " << host << "\r\n";
    mRequestStream << "Accept: */*\r\n";
    mRequestStream << "Content-Length: " << requestBody.length() << "\r\n";
    mRequestStream << "Content-Type: application/json\r\n";
    mRequestStream << "Connection: close\r\n\r\n";
    mRequestStream << requestBody;


    boost::asio::write(mSocket, mRequest);

    json result = json::parse(processResponse());
    int code = result.value("code", -1);
    if (code == 0){
        json data = result["data"];
        string ip = data.value("ip_address", "");
        int port = data.value("port", -1);
        cout << ip << " " << port;
    }
}

void UUID2IP::fetchFromGlobalCache(uuids::uuid nodeUUID) {
    mRequest.consume(mRequest.size());
    string uuid = boost::lexical_cast<string>(nodeUUID);
    string urlParams = "/api/v1/nodes/" + uuid + "/";
    string host = mServiceIP + ":" + mServicePort;
    mRequestStream << "GET " << urlParams << " HTTP/1.0\r\n";
    mRequestStream << "Host: " << host << "\r\n";
    mRequestStream << "Accept: */*\r\n";
    mRequestStream << "Connection: close\r\n\r\n";

    boost::asio::connect(mSocket, mEndpointIterator);
    boost::asio::write(mSocket, mRequest);

    json result = json::parse(processResponse());
    int code = result.value("code", -1);
    if (code == 0){
        json data = result["data"];
        string ip = data.value("ip_address", "");
        int port = data.value("port", -1);
        cout << ip << " " << port;
    }
}

string UUID2IP::processResponse() {
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



