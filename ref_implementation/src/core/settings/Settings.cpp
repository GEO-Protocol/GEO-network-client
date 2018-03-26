/**
 * This file is part of GEO Project.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Project/GEO-Project/blob/master/LICENSE.md
 *
 * No part of GEO Project, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#include "Settings.h"

json Settings::loadParsedJSON() const {
    string buffer;
    try {
        ifstream stream("conf.json");
        buffer.assign(
            (std::istreambuf_iterator<char>(stream)), // parentheses are valuable!
            std::istreambuf_iterator<char>());
        return json::parse(buffer.data());
    } catch (...) {
        throw IOError(
            "Settings::loadParsedJSON: "
                "Can't read conf.json");
    }
}

/*
 * Returns network interface of the node;
 * Throws RuntimeError in case when settings can't be read.
 */
const string Settings::interface(const json *conf) const {
    if (conf == nullptr) {
        auto j = loadParsedJSON();
        conf = &j;
    }
    try {
        return (*conf).at("network").at("interface");
    } catch (...) {
        throw RuntimeError(
            "Settings::interface: can't read node interface settings.");
    }
}

/*
 * Returns network port of the node;
 * Throws RuntimeError in case when settings can't be read.
 */
const uint16_t Settings::port(const json *conf) const {
    if (conf == nullptr) {
        auto j = loadParsedJSON();
        conf = &j;
    }
    try {
        return (*conf).at("network").at("port");
    } catch (...) {
        throw RuntimeError(
            "Settings::port: can't read node interface settings.");
    }
}

/*
 * Returns host of the uuid2address service;
 * Throws RuntimeError in case when settings can't be read.
 */
const string Settings::uuid2addressHost(const json *conf) const {
    if (conf == nullptr) {
        auto j = loadParsedJSON();
        conf = &j;
    }
    try {
        return (*conf).at("uuid2address").at("host");
    } catch (...) {
        throw RuntimeError(
            "Settings::uuid2addressHost: can't read node interface settings.");
    }
}

/*
 * Returns port of the uuid2address service;
 * Throws RuntimeError in case when settings can't be read.
 */
const uint16_t Settings::uuid2addressPort(const json *conf) const {
    if (conf == nullptr) {
        auto j = loadParsedJSON();
        conf = &j;
    }
    try {
        return (*conf).at("uuid2address").at("port");
    } catch (...) {
        throw RuntimeError(
            "Settings::uuid2addressPort: can't read node interface settings.");
    }
}

const NodeUUID Settings::nodeUUID(const json *conf) const {
    if (conf == nullptr) {
        auto j = loadParsedJSON();
        conf = &j;
    }
    try {
        string hexUUID = (*conf).at("node").at("uuid");
        return NodeUUID(hexUUID);
    } catch (...) {
        throw RuntimeError(
            "Settings::nodeUUID: can't read node uuid.");
    }
}

/*
 * Returns host of the uuid2address service;
 * Throws RuntimeError in case when settings can't be read.
 */
const uint16_t Settings::influxDbPort(const json *conf) const {
    if (conf == nullptr) {
        auto j = loadParsedJSON();
        conf = &j;
    }
    try {
        return (*conf).at("influxDB").at("port");
    } catch (...) {
        throw RuntimeError(
            "Settings::uuid2addressHost: can't read node interface settings.");
    }
}


/*
 * Returns host of the uuid2address service;
 * Throws RuntimeError in case when settings can't be read.
 */
const string Settings::influxDbHost(const json *conf) const {
    if (conf == nullptr) {
        auto j = loadParsedJSON();
        conf = &j;
    }
    try {
        return (*conf).at("influxDB").at("host");
    } catch (...) {
        throw RuntimeError(
            "Settings::uuid2addressHost: can't read node interface settings.");
    }
}

/*
 * Returns host of the uuid2address service;
 * Throws RuntimeError in case when settings can't be read.
 */
const string Settings::influxDbName(const json *conf) const {
    if (conf == nullptr) {
        auto j = loadParsedJSON();
        conf = &j;
    }
    try {
        return (*conf).at("influxDB").at("database");
    } catch (...) {
        throw RuntimeError(
            "Settings::uuid2addressHost: can't read node interface settings.");
    }
}

bool Settings::iAmGateway(const json *conf) const {
    if (conf == nullptr) {
        auto j = loadParsedJSON();
        conf = &j;
    }
    try {
        return (*conf).at("gateway");
    } catch (...) {
        // todo : throw RuntimeError
        return false;
    }
}