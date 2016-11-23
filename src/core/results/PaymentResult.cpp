#include "PaymentResult.h"

PaymentResult::PaymentResult(trust_amount amount, boost::uuids::uuid contractorUUID, Command *command,
                                             uint16_t resultCode, string timestampExcepted, string timestampCompleted) {
    mAmount = amount;
    mContractorUUID = contractorUUID;
    Result::Result(command, resultCode, timestampExcepted, timestampCompleted);
}

Command PaymentResult::getCommand() {
    return Result::getCommand();
}

uint16_t PaymentResult::getResultCode(){
    return Result::getResultCode();
}

string PaymentResult::getTimestampExcepted(){
    return Result::getTimestampExcepted();
}

string PaymentResult::getTimestampCompleted(){
    return Result::getTimestampCompleted();
}

trust_amount PaymentResult::getAmount() {
    return mAmount;
}

boost::uuids::uuid PaymentResult::getContractorUUID(){
    return mContractorUUID;
}

string PaymentResult::serialize() {
    return lexical_cast<string>(mContractorUUID) + "_" +
           lexical_cast<string>(Result::getResultCode()) + "_" +
           Result::getTimestampExcepted() + "_" +
           Result::getTimestampCompleted() + "_" +
           lexical_cast<string>(mAmount) + "\n";
}

