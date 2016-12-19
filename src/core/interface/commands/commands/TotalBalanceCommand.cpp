#include "TotalBalanceCommand.h"

TotalBalanceCommand::TotalBalanceCommand(
    const CommandUUID &uuid) :

    BaseUserCommand(uuid, kIdentifier) {}