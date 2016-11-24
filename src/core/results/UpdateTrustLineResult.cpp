#include "UpdateTrustLineResult.h"

UpdateTrustLineResult::UpdateTrustLineResult(Command *command,
                                         const uint16_t &resultCode,
                                         const string &timestampCompleted,
                                         const boost::uuids::uuid &contractorUuid,
                                         const trust_amount amount) :
        Result(command, resultCode, timestampCompleted) {
    mContractorUUID = contractorUUID;
    mAmount = amount;
}

const uint16_t &UpdateTrustLineResult::resultCode() const {
    return Result::resCode();
}

const string &UpdateTrustLineResult::timestampExcepted() const {
    return Result::exceptedTimestamp();
}

const string &UpdateTrustLineResult::timestampCompleted() const {
    return Result::completedTimestamp();
}

const boost::uuids::uuid &UpdateTrustLineResult::contractorUUID() const {
    return mContractorUuid;
}

const trust_amount &UpdateTrustLineResult::amount() const {
    return mAmount;
}

string UpdateTrustLineResult::serialize() {
    return boost::lexical_cast<string>(mContractorUuid) + "_" +
           boost::lexical_cast<string>(resultCode()) + "_" +
           timestampExcepted() + "_" +
           timestampCompleted() + "_" +
           boost::lexical_cast<string>(mAmount) + "\n";
}





