#include "CommandResult.h"

CommandResult::CommandResult(
        const CommandUUID &commandUUID,
        const uint16_t resultCode) :

        mCommandUUID(commandUUID),
        mTimestampCompleted(boost::posix_time::microsec_clock::universal_time()) {

    mResultCode = resultCode;
}

CommandResult::CommandResult(
        const CommandUUID &commandUUID,
        const uint16_t resultCode,
        string &resultInformation) :

        mCommandUUID(commandUUID),
        mTimestampCompleted(boost::posix_time::microsec_clock::universal_time()),
        mResultInformation(resultInformation) {

    mResultCode = resultCode;
}

const CommandUUID &CommandResult::commandUUID() const {
    return mCommandUUID;
}

const uint16_t CommandResult::resultCode() const {
    return mResultCode;
}

const Timestamp &CommandResult::timestampCompleted() const {
    return mTimestampCompleted;
}

const string CommandResult::serialize() const {
    if (!mResultInformation.empty()) {
        return mCommandUUID.stringUUID() + "\t" +
               boost::lexical_cast<string>(mResultCode) + "\t" +
               mResultInformation + "\n";
    }
    return mCommandUUID.stringUUID() + "\t" +
           boost::lexical_cast<string>(mResultCode) + "\n";
}