#include "TotalBalanceCommand.h"

TotalBalanceCommand::TotalBalanceCommand(
    const CommandUUID &uuid) :

    BaseUserCommand(uuid, identifier()) {}

const string &TotalBalanceCommand::identifier(){
    static const string identifier = "GET:stats/balances/total";
    return identifier;
}
