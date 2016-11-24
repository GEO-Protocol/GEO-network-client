#include "MaximalTransactionAmountResult.h"

MaximalTransactionAmountResult::MaximalTransactionAmountResult(Command *command,
                                                               const uint16_t &resultCode,
                                                               const string &timestampCompleted,
                                                               const boost::uuids::uuid &contractorUuid,
                                                               const trust_amount amount) :
        Result(command, resultCode, timestampCompleted) {
    mContractorUuid = contractorUuid;
    mAmount = amount;
}

const uint16_t &MaximalTransactionAmountResult::resultCode() const {
    return Result::resCode();
}

const string &MaximalTransactionAmountResult::timestampExcepted() const {
    return Result::exceptedTimestamp();
}

const string &MaximalTransactionAmountResult::timestampCompleted() const {
    return Result::completedTimestamp();
}

const boost::uuids::uuid &MaximalTransactionAmountResult::contractorUuid() const {
    return mContractorUuid;
}

const trust_amount &MaximalTransactionAmountResult::amount() const {
    return mAmount;
}

string MaximalTransactionAmountResult::serialize() {
    return boost::lexical_cast<string>(mContractorUuid) + "_" +
           boost::lexical_cast<string>(resultCode()) + "_" +
           timestampExcepted() + "_" +
           timestampCompleted() + "_" +
           boost::lexical_cast<string>(mAmount) + "\n";
}