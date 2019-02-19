#include "TotalBalancesCommand.h"

TotalBalancesCommand::TotalBalancesCommand(
    const CommandUUID &uuid,
    const string &commandBuffer):

    BaseUserCommand(
        uuid,
        identifier())
{
    auto check = [&](auto &ctx) { if(_attr(ctx) == '\n'){throw ValueError("TotalBalancesCommand: there is no input ");}};
    auto equivalent_add = [&](auto &ctx) { mEquivalent = _attr(ctx); };

    try
    {
        parse(commandBuffer.begin(), commandBuffer.end(), char_[check]);
        parse(commandBuffer.begin(), commandBuffer.end(), *(int_[equivalent_add]) > eol);
    }
    catch (...)
    {
        throw ValueError("TotalBalancesCommand: can't parse command");
    }
}
const string &TotalBalancesCommand::identifier()
{
    static const string identifier = "GET:stats/balance/total";
    return identifier;
}

const SerializedEquivalent TotalBalancesCommand::equivalent() const
{
    return mEquivalent;
}

CommandResult::SharedConst TotalBalancesCommand::resultOk(
    string &totalBalancesStr) const
{
    return CommandResult::SharedConst(
        new CommandResult(
            identifier(),
            UUID(),
            200,
            totalBalancesStr));
}
