#include "OpenTrustLineResult.h"

OpenTrustLineResult::OpenTrustLineResult(Command *command,
                                         const uint16_t &resultCode,
                                         const string &timestampCompleted,
                                         const NodeUUID &contractorUUID,
                                         const trust_amount amount) :
        Result(command, resultCode, timestampCompleted) {
    mContractorUUID = contractorUUID;
    mAmount = amount;
}

const uuids::uuid &OpenTrustLineResult::commandUUID() const {
    return commandsUUID();
}

const string &OpenTrustLineResult::id() const {
    return identifier();
}

const uint16_t &OpenTrustLineResult::resultCode() const {
    return resCode();
}

const string &OpenTrustLineResult::timestampExcepted() const {
    return exceptedTimestamp();
}

const string &OpenTrustLineResult::timestampCompleted() const {
    return completedTimestamp();
}

const NodeUUID &OpenTrustLineResult::contractorUUID() const {
    return mContractorUUID;
}

const trust_amount &OpenTrustLineResult::amount() const {
    return mAmount;
}

string OpenTrustLineResult::serialize() {
    return lexical_cast<string>(commandUUID()) + " " +
           lexical_cast<string>(resultCode()) +
           "\n";
}