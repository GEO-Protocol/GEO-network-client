#include "Result.h"

Result::Result(Command *command, uint16_t resultCode, string timestampExcepted, string timestampCompleted){
    mCommand = *command,
    mCode = resultCode,
    mTimestampExcepted = timestampExcepted;
    mTimestampCompleted = timestampCompleted;
}

Command Result::getCommand() {
    return mCommand;
}

uint16_t Result::getResultCode(){
    return mCode;
}

string Result::getTimestampExcepted(){
    return mTimestampExcepted;
}

string Result::getTimestampCompleted(){
    return mTimestampCompleted;
}


