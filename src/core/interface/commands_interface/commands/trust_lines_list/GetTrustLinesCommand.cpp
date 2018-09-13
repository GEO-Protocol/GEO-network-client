#include "GetTrustLinesCommand.h"

GetTrustLinesCommand::GetTrustLinesCommand(
    const CommandUUID &uuid,
    const string &commandBuffer)
    noexcept:

    BaseUserCommand(
        uuid,
        identifier())
{
    static const auto minCommandLength = 5;

    if (commandBuffer.size() < minCommandLength) {
        throw ValueError(
                "GetTrustLinesCommand: can't parse command. "
                    "Received command is to short.");
    }

    size_t tokenSeparatorPos = commandBuffer.find(
        kTokensSeparator);
    string historyFromStr = commandBuffer.substr(
        0,
        tokenSeparatorPos);
    try {
        mFrom = std::stoul(historyFromStr);
    } catch (...) {
        throw ValueError(
                "GetTrustLinesCommand: can't parse command. "
                    "Error occurred while parsing  'from' token.");
    }

    size_t nextTokenSeparatorPos = commandBuffer.find(
        kTokensSeparator,
        tokenSeparatorPos + 1);
    string historyCountStr = commandBuffer.substr(
        tokenSeparatorPos + 1,
        nextTokenSeparatorPos - tokenSeparatorPos - 1);
    try {
        mCount = std::stoul(historyCountStr);
    } catch (...) {
        throw ValueError(
                "GetTrustLinesCommand: can't parse command. "
                    "Error occurred while parsing 'count' token.");
    }

    tokenSeparatorPos = nextTokenSeparatorPos;
    nextTokenSeparatorPos = commandBuffer.size() - 1;
    string equivalentStr = commandBuffer.substr(
        tokenSeparatorPos + 1,
        nextTokenSeparatorPos - tokenSeparatorPos - 1);
    try {
        mEquivalent = (uint32_t)std::stoul(equivalentStr);
    } catch (...) {
        throw ValueError(
                "GetTrustLinesCommand: can't parse command. "
                    "Error occurred while parsing 'equivalent' token.");
    }
}

const string &GetTrustLinesCommand::identifier()
{
    static const string kIdentifier = "GET:contractors/trust-lines";
    return kIdentifier;
}

const size_t GetTrustLinesCommand::from() const
{
    return mFrom;
}

const size_t GetTrustLinesCommand::count() const
{
    return mCount;
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
