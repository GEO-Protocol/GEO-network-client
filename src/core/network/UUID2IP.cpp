#include "UUID2IP.h"

UUID2IP::UUID2IP() {
    boost::uuids::uuid contractor = boost::uuids::random_generator()();
    mCache.insert(pair < uuids::uuid, pair < string, uint16_t >> (contractor, make_pair(string("127.0.0.1"), 8086)));
}

UUID2IP::~UUID2IP() {
    mCache.clear();
}

void UUID2IP::registerInGlobalCache() {
    boost::asio::io_service io_service;
    tcp::resolver resolver(io_service);
    tcp::resolver::query query("127.0.0.1", "8088");

    tcp::socket socket(io_service);
    boost::system::error_code error = boost::asio::error::host_not_found;
    while (error && endpoint_iterator != end) {
        socket.close();
        socket.connect(*endpoint_iterator++, error);
    }
    if(error){
        throw boost::system::system_error(error);
    }

    boost::asio::streambuf request;
    std::ostream request_stream(&request);

    string raw = "{\"node_uuid\":\"13e5cf8c-5834-4e52-b65b-f9281dd1ff91\", \"ip_address\":\"198.192.0.1\", \"port\":\"8054\"}";
    request_stream << "POST / HTTP/1.1\r\n";
    request_stream << "Host: " << "localhost:3000" << "\r\n";
    request_stream << "Accept: */*\r\n";
    request_stream << "Content-Length: " <<  raw.length() << "\r\n";
    request_stream << "Content-Type: application/x-www-form-urlencoded" << "\r\n";
    request_stream << "Connection: close\r\n\r\n";
    request_stream << raw;

    boost::asio::write(socket, request);

    boost::asio::streambuf response;
    boost::asio::read_until(socket, response, "\r\n");
}

const pair <string, uint16_t> &UUID2IP::getNodeAddress(const uuids::uuid &contractorUuid) const {
    if (isNodeAddressExistInLocalCache(contractorUuid)) {
        return mCache.at(contractorUuid);
    }
    //What value must be returned in case if uuid in cache not founded
    return make_pair("", 0);
}

const bool UUID2IP::isNodeAddressExistInLocalCache(const uuids::uuid &nodeUuid) const {
    return mCache.count(nodeUuid) > 0;
}

void UUID2IP::compressLocalCache() {}

