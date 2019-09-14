#include "Settings.h"

json Settings::loadParsedJSON() const
{
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

vector<pair<string, string>> Settings::addresses(
    const json *conf) const
{
    if (conf == nullptr) {
        auto j = loadParsedJSON();
        conf = &j;
    }
    vector<pair<string, string>> result;
    try {
        auto addresses = (*conf).at("addresses");
        for (const auto &address : addresses) {
            result.emplace_back(
                address.at("type").get<string>(),
                address.at("address").get<string>());
        }
        return result;
    } catch (...) {
        // todo : throw RuntimeError
        return result;
    }
}

vector<pair<string, string>> Settings::observers(
    const json *conf) const
{
    if (conf == nullptr) {
        auto j = loadParsedJSON();
        conf = &j;
    }
    vector<pair<string, string>> result;
    try {
        auto addresses = (*conf).at("observers");
        for (const auto &address : addresses) {
            result.emplace_back(
                    address.at("type").get<string>(),
                    address.at("address").get<string>());
        }
        return result;
    } catch (...) {
        // todo : throw RuntimeError
        return result;
    }
}

vector<SerializedEquivalent> Settings::iAmGateway(
    const json *conf) const
{
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

string Settings::equivalentsRegistryAddress(
    const json *conf) const
{
    if (conf == nullptr) {
        auto j = loadParsedJSON();
        conf = &j;
    }
    try {
        auto result = (*conf).at("equivalents_registry_address").get<string>();
        return result;
    } catch (...) {
        // todo : throw RuntimeError
        return "";
    }
}

pair<string, uint16_t> Settings::interface(
    const json *conf) const
{
    if (conf == nullptr) {
        auto j = loadParsedJSON();
        conf = &j;
    }
    pair<string, uint16_t> result;
    try {
        auto interface = (*conf).at("interface");
        auto address = interface.at("host").get<string>();
        auto port = (uint16_t)interface.at("port").get<int>();
        return make_pair(
            address,
            port);
    } catch (...) {
        // todo : throw RuntimeError
        return make_pair("", 0);
    }
}

json Settings::providers(
    const json *conf) const
{
    if (conf == nullptr) {
        auto j = loadParsedJSON();
        conf = &j;
    }
    try {
        auto result = (*conf).at("providers");
        return result;
    } catch (...) {
        // todo : throw RuntimeError
        return nullptr;
    }
}

json Settings::events(
    const json *conf) const
{
    if (conf == nullptr) {
        auto j = loadParsedJSON();
        conf = &j;
    }
    try {
        auto result = (*conf).at("event_files");
        return result;
    } catch (...) {
        // todo : throw RuntimeError
        return nullptr;
    }
}