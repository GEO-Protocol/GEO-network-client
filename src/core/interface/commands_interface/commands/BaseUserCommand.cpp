#include "BaseUserCommand.h"

BaseUserCommand::BaseUserCommand(
    const CommandUUID &commandUUID,
    const string &identifier):

    mCommandUUID(commandUUID),
    mCommandIdentifier(identifier),
    mTimestampAccepted(posix::microsec_clock::universal_time()) {}


const CommandUUID &BaseUserCommand::commandUUID() const {
    return mCommandUUID;
}

const string &BaseUserCommand::commandIdentifier() const {
    return mCommandIdentifier;
}

const Timestamp &BaseUserCommand::timestampAccepted() const {
    return mTimestampAccepted;
}

const CommandResult* BaseUserCommand::unexpectedErrorResult() {
    CommandResult *result = new CommandResult(mCommandUUID, 501);
    return result;
}
