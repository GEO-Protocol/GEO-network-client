#include "ContractorsListResult.h"

ContractorsListResult::ContractorsListResult(Command *command,
                                             const uint16_t &resultCode,
                                             const string &timestampCompleted,
                                             vector<NodeUUID> &contractorsList) :
        Result(command, resultCode, timestampCompleted), mContractorsList(contractorsList) {}

const uuids::uuid &ContractorsListResult::commandUUID() const {
    return commandsUUID();
}

const string &ContractorsListResult::id() const {
    return identifier();
}

const uint16_t &ContractorsListResult::resultCode() const {
    return resCode();
}

const string &ContractorsListResult::timestampExcepted() const {
    return exceptedTimestamp();
}

const string &ContractorsListResult::timestampCompleted() const {
    return completedTimestamp();
}

const vector<NodeUUID> &ContractorsListResult::contractorsList() const {
    return mContractorsList;
}

string ContractorsListResult::serialize() {
    if (resultCode() == 200) {
        string result = boost::lexical_cast<string>(commandUUID()) + " " +
                        boost::lexical_cast<string>(resultCode()) + " ";
        for (auto &element : mContractorsList) {
            result += element.stringUUID() + " ";
        }
        result += "\n";
    }
    return boost::lexical_cast<string>(commandUUID()) + " " +
           boost::lexical_cast<string>(resultCode()) +
           "\n";
}

