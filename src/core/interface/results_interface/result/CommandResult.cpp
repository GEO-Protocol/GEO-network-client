#include "CommandResult.h"

CommandResult::CommandResult(
        const string &commandIdentifier,
        const CommandUUID &commandUUID,
        const uint16_t resultCode) :

        mCommandUUID(commandUUID),
        mTimestampCompleted(utc_now()),
        mCommandIdentifier(commandIdentifier)
{
    mResultCode = resultCode;
}

CommandResult::CommandResult(
        const string &commandIdentifier,
        const CommandUUID &commandUUID,
        const uint16_t resultCode,
        string &resultInformation) :

        mCommandUUID(commandUUID),
        mTimestampCompleted(utc_now()),
        mResultInformation(resultInformation),
        mCommandIdentifier(commandIdentifier)
{
    mResultCode = resultCode;
}

const CommandUUID &CommandResult::commandUUID() const {

    return mCommandUUID;
}

const uint16_t CommandResult::resultCode() const {

    return mResultCode;
}

const DateTime &CommandResult::timestampCompleted() const {

    return mTimestampCompleted;
}

const string CommandResult::serialize() const {

    // todo: (DM) use tokens constants instead of "\t" and "\n".
    if (!mResultInformation.empty()) {
        return mCommandUUID.stringUUID() + "\t" +
               boost::lexical_cast<string>(mResultCode) + "\t" +
               mResultInformation + "\n";
    }
    return mCommandUUID.stringUUID() + "\t" +
           boost::lexical_cast<string>(mResultCode) + "\n";
}

const string CommandResult::identifier() const {
    return mCommandIdentifier;
}