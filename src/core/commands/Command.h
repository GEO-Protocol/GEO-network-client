#ifndef GEO_NETWORK_CLIENT_COMMAND_H
#define GEO_NETWORK_CLIENT_COMMAND_H

#include <string>
#include "../common/NodeUUID.h"
#include "../trust_lines/TrustLine.h"

using namespace std;

namespace uuids = boost::uuids;

class Command {
protected:
    static const size_t kUUIDHexSize = 36;
    static const char kCommandsSeparator = '\n';
    static const char kTokensSeparator = '\r';

private:
    uuids::uuid mCommandUUID;

    string mIdentifier;

    string mTimestampExcepted;

public:
    Command(const string &identifier);

    Command(const string &identifier, const string &timestampExcepted);

    Command(const uuids::uuid &commandUUID, const string &identifier, const string &timestampExcepted);

    const string &identifier() const;

    const uuids::uuid &commandsUUID() const;

    const string &timeStampExcepted() const;

private:
    virtual void deserialize() = 0;

};


#endif //GEO_NETWORK_CLIENT_COMMAND_H
