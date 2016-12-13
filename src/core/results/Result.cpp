#include "Result.h"

Result::Result() {}

Result::Result(Command *command,
               const uint16_t &resultCode,
               const string &timestampCompleted){
    mCode = resultCode,
    //mTimestampExcepted = command->timeStampExcepted();
    mTimestampCompleted = timestampCompleted;
}

const uint16_t &Result::resCode() const{
    return mCode;
}

const string &Result::exceptedTimestamp() const{
    return mTimestampExcepted;
}

const string &Result::completedTimestamp() const{
    return mTimestampCompleted;
}


