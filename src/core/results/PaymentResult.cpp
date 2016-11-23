#include "PaymentResult.h"

PaymentResult::PaymentResult(trust_amount amount, boost::uuids::uuid contractorUUID, Command *command,
                             uint16_t resultCode, string timestampExcepted, string timestampCompleted) :
        Result(command, resultCode, timestampExcepted, timestampCompleted) {
    mAmount = amount;
    mContractorUUID = contractorUUID;
}

PaymentResult::~PaymentResult() {}

uint16_t PaymentResult::getResultCode() {
    return Result::getResCode();
}

string PaymentResult::getTimestampExcepted() {
    return Result::getExceptedTimestamp();
}

string PaymentResult::getTimestampCompleted() {
    return Result::getCompletedTimestamp();
}

trust_amount PaymentResult::getAmount() {
    return mAmount;
}

boost::uuids::uuid PaymentResult::getContractorUUID() {
    return mContractorUUID;
}

string PaymentResult::serialize() {
    return boost::lexical_cast<string>(mContractorUUID) + "_" +
           boost::lexical_cast<string>(Result::getResCode()) + "_" +
           Result::getExceptedTimestamp() + "_" +
           Result::getCompletedTimestamp() + "_" +
           boost::lexical_cast<string>(mAmount) + "\n";
}

