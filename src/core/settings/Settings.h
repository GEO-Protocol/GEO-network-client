#ifndef GEO_NETWORK_CLIENT_SETTINGS_H
#define GEO_NETWORK_CLIENT_SETTINGS_H

#include "../common/NodeUUID.h"

#include "../common/exceptions/IOError.h"
#include "../common/exceptions/RuntimeError.h"

#include "../../libs/json/json.h"

#include <string>
#include <fstream>
#include <streambuf>

using namespace std;
using namespace boost::uuids;
using json = nlohmann::json;

class Settings {
public:
    const NodeUUID nodeUUID(
        const json *conf = nullptr) const;

    const string interface(
        const json *conf = nullptr) const;

    const uint16_t port(
        const json *conf = nullptr) const;

    const string uuid2addressHost(
        const json *conf = nullptr) const;

    const uint16_t uuid2addressPort(
        const json *conf = nullptr) const;

    json loadParsedJSON() const;
};

#endif //GEO_NETWORK_CLIENT_SETTINGS_H