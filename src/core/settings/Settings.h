#ifndef GEO_NETWORK_CLIENT_SETTINGS_H
#define GEO_NETWORK_CLIENT_SETTINGS_H

#include "../common/exceptions/IOError.h"

#include "../../libs/json.h"

#include <string>
#include <fstream>
#include <streambuf>

using namespace std;
using json = nlohmann::json;

class Settings {
public:
    const string interface(const json *conf=nullptr) const;
    const uint16_t port(const json *conf=nullptr) const;
    json loadParsedJSON() const;
};


#endif //GEO_NETWORK_CLIENT_SETTINGS_H