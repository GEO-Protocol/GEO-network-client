#include "HistoryWithContractorCommand.h"

HistoryWithContractorCommand::HistoryWithContractorCommand(
    const CommandUUID &uuid,
    const string &commandBuffer):

    BaseUserCommand(
        uuid,
        identifier())
{
    const auto minCommandLength = NodeUUID::kHexSize + 5;
    if (commandBuffer.size() < minCommandLength) {
        throw ValueError(
                "HistoryWithContractorCommand: can't parse command. "
                    "Received command is to short.");
    }
    size_t tokenSeparatorPos = commandBuffer.find(
        kTokensSeparator);
    string historyFromStr = commandBuffer.substr(
        0,
        tokenSeparatorPos);
    try {
        mHistoryFrom = std::stoul(historyFromStr);
    } catch (...) {
        throw ValueError(
                "HistoryWithContractorCommand: can't parse command. "
                    "Error occurred while parsing  'from' token.");
    }

    size_t nextTokenSeparatorPos = commandBuffer.find(
        kTokensSeparator,
        tokenSeparatorPos + 1);
    string historyCountStr = commandBuffer.substr(
        tokenSeparatorPos + 1,
        nextTokenSeparatorPos - tokenSeparatorPos - 1);
    try {
        mHistoryCount = std::stoul(historyCountStr);
    } catch (...) {
        throw ValueError(
                "HistoryWithContractorCommand: can't parse command. "
                    "Error occurred while parsing 'count' token.");
    }

    tokenSeparatorPos = nextTokenSeparatorPos;
    try {
        string hexUUID = commandBuffer.substr(
            tokenSeparatorPos + 1,
            NodeUUID::kHexSize);
        mContractorUUID = boost::lexical_cast<uuids::uuid>(hexUUID);
    } catch (...) {
        throw ValueError(
                "HistoryWithContractorCommand: can't parse command. "
                    "Error occurred while parsing 'Contractor UUID' token.");
    }

    tokenSeparatorPos = tokenSeparatorPos + NodeUUID::kHexSize;
    nextTokenSeparatorPos = commandBuffer.size() - 1;
    string equivalentStr = commandBuffer.substr(
        tokenSeparatorPos + 1,
        nextTokenSeparatorPos - tokenSeparatorPos - 1);
    try {
        mEquivalent = (uint32_t)std::stoul(equivalentStr);
    } catch (...) {
        throw ValueError(
                "HistoryWithContractorCommand: can't parse command. "
                    "Error occurred while parsing 'equivalent' token.");
    }
}

const string &HistoryWithContractorCommand::identifier()
{
    static const string identifier = "GET:history/contractor";
    return identifier;
}

const size_t HistoryWithContractorCommand::historyFrom() const
{
    return mHistoryFrom;
}

const size_t HistoryWithContractorCommand::historyCount() const
{
    return mHistoryCount;
}

const NodeUUID& HistoryWithContractorCommand::contractorUUID() const
{
    return mContractorUUID;
}

const SerializedEquivalent HistoryWithContractorCommand::equivalent() const
{
    return mEquivalent;
}

CommandResult::SharedConst HistoryWithContractorCommand::resultOk(
    string &historyPaymentsStr) const
{
    return CommandResult::SharedConst(
        new CommandResult(
            identifier(),
            UUID(),
            200,
            historyPaymentsStr));
}
