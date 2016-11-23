#include "Result.h"

Result::Result() {}

Result::Result(Command *command, uint16_t resultCode, string timestampExcepted, string timestampCompleted){
    mCode = resultCode,
    mTimestampExcepted = timestampExcepted;
    mTimestampCompleted = timestampCompleted;
}

Result::~Result() {}

uint16_t Result::getResCode(){
    return mCode;
}

string Result::getExceptedTimestamp(){
    return mTimestampExcepted;
}

string Result::getCompletedTimestamp(){
    return mTimestampCompleted;
}


