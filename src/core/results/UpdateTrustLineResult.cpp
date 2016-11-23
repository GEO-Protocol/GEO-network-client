#include "UpdateTrustLineResult.h"

UpdateTrustLineResult::UpdateTrustLineResult(trust_amount amount, boost::uuids::uuid contractorUUID, Command *command,
                                             uint16_t resultCode, string timestampExcepted, string timestampCompleted) :
        Result(command, resultCode, timestampExcepted, timestampCompleted) {
    mAmount = amount;
    mContractorUUID = contractorUUID;
}

uint16_t UpdateTrustLineResult::getResultCode() {
    return Result::getResCode();
}

string UpdateTrustLineResult::getTimestampExcepted() {
    return Result::getExceptedTimestamp();
}

string UpdateTrustLineResult::getTimestampCompleted() {
    return Result::getCompletedTimestamp();
}

trust_amount UpdateTrustLineResult::getAmount() {
    return mAmount;
}

boost::uuids::uuid UpdateTrustLineResult::getContractorUUID() {
    return mContractorUUID;
}

string UpdateTrustLineResult::serialize() {
    return boost::lexical_cast<string>(mContractorUUID) + "_" +
           boost::lexical_cast<string>(Result::getResCode()) + "_" +
           Result::getExceptedTimestamp() + "_" +
           Result::getCompletedTimestamp() + "_" +
           boost::lexical_cast<string>(mAmount) + "\n";
}





