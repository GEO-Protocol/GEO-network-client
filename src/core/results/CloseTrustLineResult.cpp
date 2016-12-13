#include "CloseTrustLineResult.h"

CloseTrustLineResult::CloseTrustLineResult(Command *command,
                                         const uint16_t &resultCode,
                                         const string &timestampCompleted,
                                         const NodeUUID &contractorUUID) :
        Result(command, resultCode, timestampCompleted) {
    mContractorUUID = contractorUUID;
}

const uuids::uuid &CloseTrustLineResult::commandUUID() const {
    return commandsUUID();
}

const string &CloseTrustLineResult::id() const {
    return identifier();
}

const uint16_t &CloseTrustLineResult::resultCode() const {
    return resCode();
}

const string &CloseTrustLineResult::timestampExcepted() const {
    return exceptedTimestamp();
}

const string &CloseTrustLineResult::timestampCompleted() const {
    return completedTimestamp();
}

const NodeUUID &CloseTrustLineResult::contractorUUID() const {
    return mContractorUUID;
}

string CloseTrustLineResult::serialize() {
    return boost::lexical_cast<string>(commandUUID()) + " " +
           boost::lexical_cast<string>(resultCode()) +
           "\n";
}


