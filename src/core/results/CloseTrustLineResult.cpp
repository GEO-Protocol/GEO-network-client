#include "CloseTrustLineResult.h"

CloseTrustLineResult::CloseTrustLineResult(boost::uuids::uuid contractorUUID, Command *command,
                                           uint16_t resultCode, string timestampExcepted, string timestampCompleted) :
        Result(command, resultCode, timestampExcepted, timestampCompleted) {
    mContractorUUID = contractorUUID;
}

CloseTrustLineResult::~CloseTrustLineResult() {}

uint16_t CloseTrustLineResult::getResultCode() {
    return Result::getResCode();
}

string CloseTrustLineResult::getTimestampExcepted() {
    return Result::getExceptedTimestamp();
}

string CloseTrustLineResult::getTimestampCompleted() {
    return Result::getCompletedTimestamp();
}

boost::uuids::uuid CloseTrustLineResult::getContractorUUID() {
    return mContractorUUID;
}

string CloseTrustLineResult::serialize() {
    return boost::lexical_cast<string>(mContractorUUID) + "_" +
           boost::lexical_cast<string>(Result::getResCode()) + "_" +
           Result::getExceptedTimestamp() + "_" +
           Result::getCompletedTimestamp() + "\n";
}


