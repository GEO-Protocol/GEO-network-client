#include "CloseTrustLineResult.h"

CloseTrustLineResult::CloseTrustLineResult(Command *command,
                                         const uint16_t &resultCode,
                                         const string &timestampCompleted,
                                         const boost::uuids::uuid &contractorUuid) :
        Result(command, resultCode, timestampCompleted) {
    mContractorUUID = contractorUUID;
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

const boost::uuids::uuid &CloseTrustLineResult::contractorUUID() const {
    return mContractorUuid;
}

string OpenTrustLineResult::serialize() {
    return boost::lexical_cast<string>(mContractorUuid) + "_" +
           boost::lexical_cast<string>(resultCode()) + "_" +
           timestampExcepted() + "_" +
           timestampCompleted() + "\n";
}


