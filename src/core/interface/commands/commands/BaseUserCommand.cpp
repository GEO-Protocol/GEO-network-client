#include "BaseUserCommand.h"

BaseUserCommand::BaseUserCommand(
    const CommandUUID &uuid,
    const string &identifier):

    mUUID(uuid),
    mDerivedIdentifier(identifier),
    mTimestampAccepted(boost::posix_time::microsec_clock::universal_time()) {}

const CommandUUID &BaseUserCommand::uuid() const {
    return mUUID;
}

const Timestamp &BaseUserCommand::timestampAccepted() const {
    return mTimestampAccepted;
}

const string &BaseUserCommand::derivedIdentifier() const {
    return mDerivedIdentifier;
}

CommandResult::SharedConst BaseUserCommand::unexpectedErrorResult() {
    CommandResult *result = new CommandResult(mUUID, 501);
    return CommandResult::SharedConst(result);
}
