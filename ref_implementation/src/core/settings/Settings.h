/**
 * This file is part of GEO Project.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Project/GEO-Project/blob/master/LICENSE.md
 *
 * No part of GEO Project, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

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

    const string influxDbName(
        const json *conf = nullptr) const;

    const string influxDbHost(
        const json *conf = nullptr) const;

    const uint16_t influxDbPort(
        const json *conf = nullptr) const;

    bool iAmGateway(
        const json *conf = nullptr) const;

    json loadParsedJSON() const;
};

#endif //GEO_NETWORK_CLIENT_SETTINGS_H