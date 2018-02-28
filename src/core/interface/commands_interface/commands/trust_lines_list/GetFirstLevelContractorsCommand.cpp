#include "GetFirstLevelContractorsCommand.h"

GetFirstLevelContractorsCommand::GetFirstLevelContractorsCommand(
    const CommandUUID &uuid,
    const string& commandBuffer)
    noexcept:

    BaseUserCommand(
        uuid,
        identifier())
{
    static const auto minCommandLength = 1;

    if (commandBuffer.size() < minCommandLength) {
        throw ValueError(
                "GetFirstLevelContractorsCommand: can't parse command. "
                    "Received command is to short.");
    }

    string equivalentStr = commandBuffer.substr(
        0,
        commandBuffer.size() - 1);
    try {
        mEquivalent = (uint32_t)std::stoul(equivalentStr);
    } catch (...) {
        throw ValueError(
                "GetFirstLevelContractorsCommand: can't parse command. "
                    "Error occurred while parsing  'equivalent' token.");
    }
}

const string &GetFirstLevelContractorsCommand::identifier()
{
    static const string identifier = "GET:contractors";
    return identifier;
}

const SerializedEquivalent GetFirstLevelContractorsCommand::equivalent() const
{
    return mEquivalent;
}

CommandResult::SharedConst GetFirstLevelContractorsCommand::resultOk(
    string& neighbors) const
{
    return make_shared<const CommandResult>(
        identifier(),
        UUID(),
        200,
        neighbors);
}

