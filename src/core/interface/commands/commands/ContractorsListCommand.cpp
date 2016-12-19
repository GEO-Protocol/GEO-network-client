#include "ContractorsListCommand.h"


ContractorsListCommand::ContractorsListCommand(
    const CommandUUID &uuid) :

    BaseUserCommand(uuid, kIdentifier) {}
