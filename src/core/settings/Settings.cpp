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

const string Settings::interface(const json *conf) const {
    if (conf == nullptr) {
        auto j = loadParsedJSON();
        conf = &j;
    }

    return (*conf)["network"]["interface"];
}

const uint16_t Settings::port(const json *conf) const {
    if (conf == nullptr) {
        auto j = loadParsedJSON();
        conf = &j;
    }

    return (*conf)["network"]["port"];
}
