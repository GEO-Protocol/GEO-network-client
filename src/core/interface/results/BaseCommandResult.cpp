#include "BaseCommandResult.h"

BaseCommandResult::BaseCommandResult(
    const uint16_t &resultCode):

    mResultCode(resultCode),
    mTimestampCompleted(boost::posix_time::microsec_clock::local_time()){}

const uint16_t BaseCommandResult::resultCode() const {
    return mResultCode;
}

const Timestamp &BaseCommandResult::timestampCompleted() const {
    return mTimestampCompleted;
}
