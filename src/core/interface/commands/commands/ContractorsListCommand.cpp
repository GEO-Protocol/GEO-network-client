#include "ContractorsListCommand.h"


ContractorsListCommand::ContractorsListCommand(
    const CommandUUID &uuid) :

    BaseUserCommand(uuid, identifier()) {}

const string &ContractorsListCommand::identifier(){
    static const string identifier = "GET:contractors";
    return identifier;
}
