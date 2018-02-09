#ifndef GEO_NETWORK_CLIENT_BASECOMMANDRESULT_H
#define GEO_NETWORK_CLIENT_BASECOMMANDRESULT_H

#include "../../../common/Types.h"
#include "../../../common/time/TimeUtils.h"

#include "../../commands_interface/CommandUUID.h"

#include <string>
#include <memory>

using namespace std;

class CommandResult {
public:
    typedef shared_ptr<CommandResult> Shared;
    typedef shared_ptr<const CommandResult> SharedConst;

public:
    enum CommandResultCode {
        OK = 200,

        // todo: add codes from specification
    };

public:
    CommandResult(
        const string &commandIdentifier,
        const CommandUUID &commandUUID,
        const uint16_t resultCode);

    CommandResult(
        const string &commandIdentifier,
        const CommandUUID &commandUUID,
        const uint16_t resultCode,
        string &resultInformation);

    const CommandUUID &commandUUID() const;

    const uint16_t resultCode() const;

    const DateTime &timestampCompleted() const;

    const string serialize() const;

    const string serializeShort() const;

    const string identifier() const;

private:
    CommandUUID mCommandUUID;
    uint16_t mResultCode;
    DateTime mTimestampCompleted;
    string mResultInformation;
    string mCommandIdentifier;
};


#endif //GEO_NETWORK_CLIENT_BASECOMMANDRESULT_H
