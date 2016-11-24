#include "CloseTrustLineResult.h"

CloseTrustLineResult::CloseTrustLineResult(Command *command,
                                         const uint16_t &resultCode,
                                         const string &timestampCompleted,
                                         const boost::uuids::uuid &contractorUuid) :
        Result(command, resultCode, timestampCompleted) {
    mContractorUuid = contractorUuid;
}

const uint16_t &CloseTrustLineResult::resultCode() const {
    return Result::resCode();
}

const string &CloseTrustLineResult::timestampExcepted() const {
    return Result::exceptedTimestamp();
}

const string &CloseTrustLineResult::timestampCompleted() const {
    return Result::completedTimestamp();
}

const boost::uuids::uuid &CloseTrustLineResult::contractorUuid() const {
    return mContractorUuid;
}

string CloseTrustLineResult::serialize() {
    return boost::lexical_cast<string>(mContractorUuid) + "_" +
           boost::lexical_cast<string>(resultCode()) + "_" +
           timestampExcepted() + "_" +
           timestampCompleted() + "\n";
}


