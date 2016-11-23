#include "OpenTrustLineResult.h"

OpenTrustLineResult::OpenTrustLineResult(trust_amount amount, boost::uuids::uuid contractorUUID, Command *command,
                                         uint16_t resultCode, string timestampExcepted, string timestampCompleted) {
    mAmount = amount;
    mContractorUUID = contractorUUID;
    Result::Result(command, resultCode, timestampExcepted, timestampCompleted);
}

Command OpenTrustLineResult::getCommand() {
    return Result::getCommand();
}

uint16_t OpenTrustLineResult::getResultCode() {
    return Result::getResultCode();
}

string OpenTrustLineResult::getTimestampExcepted() {
    return Result::getTimestampExcepted();
}

string OpenTrustLineResult::getTimestampCompleted() {
    return Result::getTimestampCompleted();
}

trust_amount OpenTrustLineResult::getAmount() {
    return mAmount;
}

boost::uuids::uuid OpenTrustLineResult::getContractorUUID() {
    return mContractorUUID;
}

string OpenTrustLineResult::serialize() {
    return lexical_cast<string>(mContractorUUID) + "_" +
           lexical_cast<string>(Result::getResultCode()) + "_" +
           Result::getTimestampExcepted() + "_" +
           Result::getTimestampCompleted() + "_" +
           lexical_cast<string>(mAmount) + "\n";
}



