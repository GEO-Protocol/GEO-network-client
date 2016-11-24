#include "Command.h"

Command::Command(const string &identifier) :
        mIdentifier(identifier) {}

Command::Command(const string &identifier, const string &timestampExcepted) :
        mIdentifier(identifier), mTimestampExcepted(timestampExcepted) {}

const string &Command::identifier() const {
    return mIdentifier;
}

const string &Command::timeStampExcepted() const {
    return mTimestampExcepted;
}