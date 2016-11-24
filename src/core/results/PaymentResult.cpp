#include "PaymentResult.h"

PaymentResult::PaymentResult(Command *command,
                             const uint16_t &resultCode,
                             const string &timestampCompleted,
                             const boost::uuids::uuid &contractorUuid,
                             const trust_amount amount) :
        Result(command, resultCode, timestampCompleted) {
    mContractorUuid = contractorUuid;
    mAmount = amount;
}

const uint16_t &PaymentResult::resultCode() const {
    return Result::resCode();
}

const string &PaymentResult::timestampExcepted() const {
    return Result::exceptedTimestamp();
}

const string &PaymentResult::timestampCompleted() const {
    return Result::completedTimestamp();
}

const boost::uuids::uuid &PaymentResult::contractorUuid() const {
    return mContractorUuid;
}

const trust_amount &PaymentResult::amount() const {
    return mAmount;
}

string PaymentResult::serialize() {
    return boost::lexical_cast<string>(mContractorUuid) + "_" +
           boost::lexical_cast<string>(resultCode()) + "_" +
           timestampExcepted() + "_" +
           timestampCompleted() + "_" +
           boost::lexical_cast<string>(mAmount) + "\n";
}

