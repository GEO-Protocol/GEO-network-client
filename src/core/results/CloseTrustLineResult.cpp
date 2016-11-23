#include "CloseTrustLineResult.h"

CloseTrustLineResult::CloseTrustLineResult(boost::uuids::uuid contractorUUID, Command *command,
                             uint16_t resultCode, string timestampExcepted, string timestampCompleted) {
    mContractorUUID = contractorUUID;
    Result::Result(command, resultCode, timestampExcepted, timestampCompleted);
}

Command CloseTrustLineResult::getCommand() {
    return Result::getCommand();
}

uint16_t CloseTrustLineResult::getResultCode(){
    return Result::getResultCode();
}

string CloseTrustLineResult::getTimestampExcepted(){
    return Result::getTimestampExcepted();
}

string CloseTrustLineResult::getTimestampCompleted(){
    return Result::getTimestampCompleted();
}

boost::uuids::uuid CloseTrustLineResult::getContractorUUID(){
    return mContractorUUID;
}

string CloseTrustLineResult::serialize() {
    return lexical_cast<string>(mContractorUUID) + "_" +
           lexical_cast<string>(Result::getResultCode()) + "_" +
           Result::getTimestampExcepted() + "_" +
           Result::getTimestampCompleted() + "\n";
}

