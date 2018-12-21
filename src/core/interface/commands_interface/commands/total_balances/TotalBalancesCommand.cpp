#include "TotalBalancesCommand.h"

TotalBalancesCommand::TotalBalancesCommand(
    const CommandUUID &uuid,
    const string &commandBuffer):

    BaseUserCommand(
        uuid,
        identifier())
{
    const auto minCommandLength = 1;
    if (commandBuffer.size() < minCommandLength) {
        throw ValueError(
                "TotalBalancesCommand: can't parse command. "
                    "Received command is to short.");
    }

    string equivalentStr = commandBuffer.substr(
        0,
        commandBuffer.size() - 1);
    try {
        mEquivalent = (uint32_t)std::stoul(equivalentStr);
    } catch (...) {
        throw ValueError(
                "TotalBalancesCommand: can't parse command. "
                    "Error occurred while parsing  'equivalent' token.");
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
