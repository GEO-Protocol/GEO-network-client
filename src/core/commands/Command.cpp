#include "Command.h"

Command::Command(const string &identifier) :
        mIdentifier(identifier) {}

Command::Command(const string &identifier, const string &timestampExcepted) :
        mIdentifier(identifier), mTimestampExcepted(timestampExcepted) {}

Command::Command(const uuids::uuid &commandUUID, const string &identifier, const string &timestampExcepted) :
        mCommandUUID(commandUUID), mIdentifier(identifier), mTimestampExcepted(timestampExcepted) {}

const uuids::uuid &Command::commandsUUID() const {
    return mCommandUUID;
}

const string &Command::identifier() const {
    return mIdentifier;
}

const string &Command::timeStampExcepted() const {
    return mTimestampExcepted;
}