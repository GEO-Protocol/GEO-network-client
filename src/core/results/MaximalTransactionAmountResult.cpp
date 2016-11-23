#include "MaximalTransactionAmountResult.h"

MaximalTransactionAmountResult::MaximalTransactionAmountResult(trust_amount amount, boost::uuids::uuid contractorUUID,
                                                               Command *command,
                                                               uint16_t resultCode,
                                                               string timestampExcepted,
                                                               string timestampCompleted) :
        Result(command, resultCode, timestampExcepted, timestampCompleted) {
    mAmount = amount;
    mContractorUUID = contractorUUID;
}

MaximalTransactionAmountResult::~MaximalTransactionAmountResult() {}

uint16_t MaximalTransactionAmountResult::getResultCode() {
    return Result::getResCode();
}

string MaximalTransactionAmountResult::getTimestampExcepted() {
    return Result::getExceptedTimestamp();
}

string MaximalTransactionAmountResult::getTimestampCompleted() {
    return Result::getCompletedTimestamp();
}

trust_amount MaximalTransactionAmountResult::getAmount() {
    return mAmount;
}

boost::uuids::uuid MaximalTransactionAmountResult::getContractorUUID() {
    return mContractorUUID;
}

string MaximalTransactionAmountResult::serialize() {
    return boost::lexical_cast<string>(mContractorUUID) + "_" +
           boost::lexical_cast<string>(Result::getResCode()) + "_" +
           Result::getExceptedTimestamp() + "_" +
           Result::getCompletedTimestamp() + "_" +
           boost::lexical_cast<string>(mAmount) + "\n";
}


