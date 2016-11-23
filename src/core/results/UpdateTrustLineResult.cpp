#include "UpdateTrustLineResult.h"

UpdateTrustLineResult::UpdateTrustLineResult(trust_amount amount, boost::uuids::uuid contractorUUID, Command *command,
                                         uint16_t resultCode, string timestampExcepted, string timestampCompleted) {
    mAmount = amount;
    mContractorUUID = contractorUUID;
    Result::Result(command, resultCode, timestampExcepted, timestampCompleted);
}

Command UpdateTrustLineResult::getCommand() {
    return Result::getCommand();
}

uint16_t UpdateTrustLineResult::getResultCode(){
    return Result::getResultCode();
}

string UpdateTrustLineResult::getTimestampExcepted(){
    return Result::getTimestampExcepted();
}

string UpdateTrustLineResult::getTimestampCompleted(){
    return Result::getTimestampCompleted();
}

trust_amount UpdateTrustLineResult::getAmount() {
    return mAmount;
}

boost::uuids::uuid UpdateTrustLineResult::getContractorUUID(){
    return mContractorUUID;
}

string PaymentResult::serialize() {
    return lexical_cast<string>(mContractorUUID) + "_" +
           lexical_cast<string>(Result::getResultCode()) + "_" +
           Result::getTimestampExcepted() + "_" +
           Result::getTimestampCompleted() + "_" +
           lexical_cast<string>(mAmount) + "\n";
}





