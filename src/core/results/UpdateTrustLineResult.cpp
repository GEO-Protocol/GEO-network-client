#include "UpdateTrustLineResult.h"

UpdateTrustLineResult::UpdateTrustLineResult(Command *command,
                                         const uint16_t &resultCode,
                                         const string &timestampCompleted,
                                         const NodeUUID &contractorUUID,
                                         const trust_amount amount) :
        Result(command, resultCode, timestampCompleted) {
    mContractorUUID = contractorUUID;
    mAmount = amount;
}

const uuids::uuid &UpdateTrustLineResult::commandUUID() const {
    return commandsUUID();
}

const string &UpdateTrustLineResult::id() const {
    return identifier();
}

const uint16_t &UpdateTrustLineResult::resultCode() const {
    return resCode();
}

const string &UpdateTrustLineResult::timestampExcepted() const {
    return exceptedTimestamp();
}

const string &UpdateTrustLineResult::timestampCompleted() const {
    return completedTimestamp();
}

const NodeUUID &UpdateTrustLineResult::contractorUUID() const {
    return mContractorUUID;
}

const trust_amount &UpdateTrustLineResult::amount() const {
    return mAmount;
}

string UpdateTrustLineResult::serialize() {
    return lexical_cast<string>(commandUUID()) + " " +
           lexical_cast<string>(resultCode()) +
           "\n";
}





