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

vector<SerializedEquivalent> Settings::iAmGateway(const json *conf) const {
    if (conf == nullptr) {
        auto j = loadParsedJSON();
        conf = &j;
    }
    vector<SerializedEquivalent> result;
    try {
        result = (*conf).at("gateway").get<vector<SerializedEquivalent>>();
        return result;
    } catch (...) {
        // todo : throw RuntimeError
        return result;
    }
}