#include "Result.h"

Result::Result(BaseUserCommand *command,
               const uint16_t &resultCode,
               const string &timestampCompleted) :
        mCommandUUID(command->uuid()), mIdentifier(command->identifier()),
        mTimestampExcepted(mTimestampExcepted), mTimestampCompleted(timestampCompleted) {
    mCode = resultCode;
}

const uuids::uuid &Result::commandsUUID() const {
    return mCommandUUID;
}

const string &Result::identifier() const {
    return mIdentifier;
}

const uint16_t &Result::resCode() const {
    return mCode;
}

const string &Result::exceptedTimestamp() const {
    return mTimestampExcepted;
}

const string &Result::completedTimestamp() const {
    return mTimestampCompleted;
}


