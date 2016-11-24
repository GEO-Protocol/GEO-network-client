#include "OpenTrustLineResult.h"

OpenTrustLineResult::OpenTrustLineResult(Command *command,
                                         const uint16_t &resultCode,
                                         const string &timestampCompleted,
                                         const boost::uuids::uuid &contractorUuid,
                                         const trust_amount amount) :
        Result(command, resultCode, timestampCompleted) {
    mContractorUuid = contractorUuid;
    mAmount = amount;
}

const uint16_t &OpenTrustLineResult::resultCode() const {
    return Result::resCode();
}

const string &OpenTrustLineResult::timestampExcepted() const {
    return Result::exceptedTimestamp();
}

const string &OpenTrustLineResult::timestampCompleted() const {
    return Result::completedTimestamp();
}

const boost::uuids::uuid &OpenTrustLineResult::contractorUuid() const {
    return mContractorUuid;
}

const trust_amount &OpenTrustLineResult::amount() const {
    return mAmount;
}

string OpenTrustLineResult::serialize() {
    return boost::lexical_cast<string>(mContractorUuid) + "_" +
           boost::lexical_cast<string>(resultCode()) + "_" +
           timestampExcepted() + "_" +
           timestampCompleted() + "_" +
           boost::lexical_cast<string>(mAmount) + "\n";
}