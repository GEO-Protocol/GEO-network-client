#include "OpenTrustLineResult.h"

OpenTrustLineResult::OpenTrustLineResult(trust_amount amount, boost::uuids::uuid contractorUUID, Command *command,
                                         uint16_t resultCode, string timestampExcepted, string timestampCompleted) :
        Result(command, resultCode, timestampExcepted, timestampCompleted) {
    mAmount = amount;
    mContractorUUID = contractorUUID;
}

OpenTrustLineResult::~OpenTrustLineResult() {}

uint16_t OpenTrustLineResult::getResultCode() {
    return Result::getResCode();
}

string OpenTrustLineResult::getTimestampExcepted() {
    return Result::getExceptedTimestamp();
}

string OpenTrustLineResult::getTimestampCompleted() {
    return Result::getCompletedTimestamp();
}

trust_amount OpenTrustLineResult::getAmount() {
    return mAmount;
}

boost::uuids::uuid OpenTrustLineResult::getContractorUUID() {
    return mContractorUUID;
}

string OpenTrustLineResult::serialize() {
    return boost::lexical_cast<string>(mContractorUUID) + "_" +
           boost::lexical_cast<string>(Result::getResCode()) + "_" +
           Result::getExceptedTimestamp() + "_" +
           Result::getCompletedTimestamp() + "_" +
           boost::lexical_cast<string>(mAmount) + "\n";
}




