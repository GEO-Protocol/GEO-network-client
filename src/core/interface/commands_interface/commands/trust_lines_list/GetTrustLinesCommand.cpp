#include "GetTrustLinesCommand.h"

GetTrustLinesCommand::GetTrustLinesCommand(
    const CommandUUID &uuid,
    const string &commandBuffer)
    noexcept:

    BaseUserCommand(
        uuid,
        identifier())
{
    static const auto minCommandLength = 1;

    if (commandBuffer.size() < minCommandLength) {
        throw ValueError(
                "GetTrustLinesCommand: can't parse command. "
                    "Received command is to short.");
    }

    string equivalentStr = commandBuffer.substr(
        0,
        commandBuffer.size() - 1);
    try {
        mEquivalent = (uint32_t)std::stoul(equivalentStr);
    } catch (...) {
        throw ValueError(
                "GetTrustLinesCommand: can't parse command. "
                    "Error occurred while parsing  'equivalent' token.");
    }
}

const string &GetTrustLinesCommand::identifier()
{
    static const string kIdentifier = "GET:contractors/trust-lines";
    return kIdentifier;
}

const SerializedEquivalent GetTrustLinesCommand::equivalent() const
{
    return mEquivalent;
}

CommandResult::SharedConst GetTrustLinesCommand::resultOk(
    string &neighbors) const
{
    return make_shared<const CommandResult>(
        identifier(),
        UUID(),
        200,
        neighbors);
}
