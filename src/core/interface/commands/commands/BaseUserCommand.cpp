#include <boost/date_time/posix_time/posix_time_types.hpp>
#include "BaseUserCommand.h"


BaseUserCommand::BaseUserCommand(
    const CommandUUID &uuid,
    const string &identifier):

    mUUID(uuid),
    mDerivedIdentifier(identifier),
    mTimestampAccepted(boost::posix_time::microsec_clock::local_time()) {}

const CommandUUID &BaseUserCommand::uuid() const {
    return mUUID;
}

const Timestamp &BaseUserCommand::timestampAccepted() const {
    return mTimestampAccepted;
}

const string &BaseUserCommand::derivedIdentifier() const {
    return mDerivedIdentifier;
}
