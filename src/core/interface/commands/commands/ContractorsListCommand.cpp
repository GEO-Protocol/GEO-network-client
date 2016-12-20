#include "ContractorsListCommand.h"


ContractorsListCommand::ContractorsListCommand(
    const CommandUUID &uuid) :

    BaseUserCommand(uuid, identifier()) {}

const string &ContractorsListCommand::identifier(){
    static const string identifier = "GET:contractors";
    return identifier;
}

const CommandResult *ContractorsListCommand::resultOk(vector<NodeUUID> contractors) const {
    string additionalInformation;
    for (auto &contractor : contractors) {
        additionalInformation += contractor.stringUUID();
        if (contractor != contractors.at(contractors.size() - 1)){
            additionalInformation += "\r";
        }
    }
    return new CommandResult(uuid(), 200);
}

const CommandResult *ContractorsListCommand::noContractorsResult() const {
    return new CommandResult(uuid(), 404);
}
