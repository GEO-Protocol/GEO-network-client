#include "TotalBalancesCommand.h"

TotalBalancesCommand::TotalBalancesCommand(
    const CommandUUID &uuid,
    const string &commandBuffer):

    BaseUserCommand(
        uuid,
        identifier())
{
    parse(commandBuffer);
}

const string &TotalBalancesCommand::identifier()
{
    static const string identifier = "GET:stats/balance/total";
    return identifier;
}

void TotalBalancesCommand::parse(
    const string &command)
{}

CommandResult::SharedConst TotalBalancesCommand::resultOk(
    string &totalBalancesStr) const
{
    return CommandResult::SharedConst(
        new CommandResult(
            UUID(),
            200,
            totalBalancesStr));
}
