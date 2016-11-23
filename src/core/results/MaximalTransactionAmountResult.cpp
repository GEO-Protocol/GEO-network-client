#include "MaximalTransactionAmountResult.h"

MaximalTransactionAmountResult::MaximalTransactionAmountResult(trust_amount amount, boost::uuids::uuid contractorUUID, Command *command,
                             uint16_t resultCode, string timestampExcepted, string timestampCompleted) {
    mAmount = amount;
    mContractorUUID = contractorUUID;
    Result::Result(command, resultCode, timestampExcepted, timestampCompleted);
}

Command MaximalTransactionAmountResult::getCommand() {
    return Result::getCommand();
}

uint16_t MaximalTransactionAmountResult::getResultCode(){
    return Result::getResultCode();
}

string MaximalTransactionAmountResult::getTimestampExcepted(){
    return Result::getTimestampExcepted();
}

string MaximalTransactionAmountResult::getTimestampCompleted(){
    return Result::getTimestampCompleted();
}

trust_amount MaximalTransactionAmountResult::getAmount() {
    return mAmount;
}

boost::uuids::uuid MaximalTransactionAmountResult::getContractorUUID(){
    return mContractorUUID;
}

string MaximalTransactionAmountResult::serialize() {
    return lexical_cast<string>(mContractorUUID) + "_" +
           lexical_cast<string>(Result::getResultCode()) + "_" +
           Result::getTimestampExcepted() + "_" +
           Result::getTimestampCompleted() + "_" +
           lexical_cast<string>(mAmount) + "\n";
}

