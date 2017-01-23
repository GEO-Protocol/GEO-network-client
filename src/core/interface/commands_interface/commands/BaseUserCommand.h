#ifndef GEO_NETWORK_CLIENT_COMMAND_H
#define GEO_NETWORK_CLIENT_COMMAND_H

#include "../../../common/Types.h"

#include "../CommandUUID.h"

#include "../../results_interface/result/CommandResult.h"

#include <boost/uuid/uuid.hpp>

#include <string>

namespace uuids = boost::uuids;

class BaseUserCommand {
public:
    typedef shared_ptr<BaseUserCommand> Shared;

public:
    BaseUserCommand(
        const CommandUUID &commandUUID,
        const string& identifier);

    const CommandUUID &commandUUID() const;

    const string &commandIdentifier() const;

    const Timestamp &timestampAccepted() const;

    const CommandResult *unexpectedErrorResult();

protected:
    virtual void deserialize(
        const string &commandBuffer) = 0;

public:
    static const constexpr char kCommandsSeparator = '\n';
    static const constexpr char kTokensSeparator = '\t';

protected:
    const CommandUUID mCommandUUID;
    const Timestamp mTimestampAccepted;

private:
    const string mCommandIdentifier;

};


#endif //GEO_NETWORK_CLIENT_COMMAND_H
