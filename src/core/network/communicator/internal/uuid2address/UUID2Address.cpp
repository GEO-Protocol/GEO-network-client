#include "UUID2Address.h"
#include "../../../../common/exceptions/NotFoundError.h"

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
    mRequestStream(&mRequest)
{
    mEndpointIterator = mResolver.resolve(mQuery);
}

UUID2Address::~UUID2Address()
{
    mRequest.consume(mRequest.size());
    mSocket.close();
}

void UUID2Address::registerInGlobalCache(
    const NodeUUID &uuid,
    const string &nodeHost,
    const uint16_t &nodePort)
{
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

UDPEndpoint& UUID2Address::fetchFromGlobalCache(
    const NodeUUID &uuid)
{
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
        json data = json::parse(result.second)["data"];
        string address = data.value("ip_address", "");
        uint16_t port = static_cast<uint16_t>(data.value("port", -1));

        mCache[uuid] = as::ip::udp::endpoint(
            as::ip::address_v4::from_string(address),
            port);

        mLastAccessTime[uuid] = chrono::steady_clock::now();
        return mCache[uuid];
    }

    default: {
        throw NotFoundError(
            "UUID2Address::fetchFromGlobalCache: "
            "One or sevaral request parameters are invalid.");
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

UDPEndpoint& UUID2Address::endpoint (
    const NodeUUID &contractorUUID)
{
    if (mCache.count(contractorUUID) > 0) {
        mLastAccessTime[contractorUUID] = chrono::steady_clock::now();
        return mCache[contractorUUID];
    }

    return fetchFromGlobalCache(contractorUUID);
}
