#ifndef GEO_NETWORK_CLIENT_RESULT_H
#define GEO_NETWORK_CLIENT_RESULT_H

#include <string>
#include <vector>
#include "../trust_lines/TrustLine.h"
#include "../common/NodeUUID.h"
#include "../commands/Command.h"

using namespace std;

class Result {
private:
    uuids::uuid mCommandUUID;
    string mIdentifier;
    uint16_t mCode;
    string mTimestampExcepted;
    string mTimestampCompleted;

public:
    Result(Command *command,
           const uint16_t &resultCode,
           const string &timestampCompleted);

    const uuids::uuid &commandsUUID() const;

    const string &identifier() const;

    const uint16_t &resCode() const;

    const string &exceptedTimestamp() const;

    const string &completedTimestamp() const;

    virtual string serialize() = 0;
};

#endif //GEO_NETWORK_CLIENT_RESULT_H
