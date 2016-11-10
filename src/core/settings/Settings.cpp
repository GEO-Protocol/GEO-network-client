#include "Settings.h"

json Settings::loadParsedJSON() const {
    string buffer;

    try {
        ifstream stream("conf.json");

        stream.seekg(0, std::ios::end);
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wsign-conversion"
        buffer.reserve(stream.tellg());
#pragma clang diagnostic pop
        stream.seekg(0, std::ios::beg);

        buffer.assign((std::istreambuf_iterator<char>(stream)),
                   std::istreambuf_iterator<char>());

    } catch (...) {
        throw IOError("Can't read conf.json");
    }

    try {
        return json::parse(buffer.data());

    } catch (...) {
        throw IOError("Can't parse conf.json");
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
