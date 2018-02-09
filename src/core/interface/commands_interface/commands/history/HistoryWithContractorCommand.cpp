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
        throw ValueError("HistoryWithContractorCommand::parse: "
                             "Can't parse command. Received command is to short.");
    }
    size_t tokenSeparatorPos = commandBuffer.find(
        kTokensSeparator);
    string historyFromStr = commandBuffer.substr(
        0,
        tokenSeparatorPos);
    if (historyFromStr.at(0) == '-') {
        throw ValueError("HistoryWithContractorCommand::parse: "
                             "Can't parse command. 'from' token can't be negative.");
    }
    try {
        mHistoryFrom = std::stoul(historyFromStr);
    } catch (...) {
        throw ValueError("HistoryWithContractorCommand::parse: "
                             "Can't parse command. Error occurred while parsing  'from' token.");
    }

    size_t nextTokenSeparatorPos = commandBuffer.find(
        kTokensSeparator,
        tokenSeparatorPos + 1);
    string historyCountStr = commandBuffer.substr(
        tokenSeparatorPos + 1,
        nextTokenSeparatorPos - tokenSeparatorPos - 1);
    if (historyCountStr.at(0) == '-') {
        throw ValueError("HistoryWithContractorCommand::parse: "
                             "Can't parse command. 'count' token can't be negative.");
    }
    try {
        mHistoryCount = std::stoul(historyCountStr);
    } catch (...) {
        throw ValueError("HistoryWithContractorCommand::parse: "
                             "Can't parse command. Error occurred while parsing 'count' token.");
    }

    tokenSeparatorPos = nextTokenSeparatorPos;
    try {
        string hexUUID = commandBuffer.substr(
            tokenSeparatorPos + 1,
            NodeUUID::kHexSize);
        mContractorUUID = boost::lexical_cast<uuids::uuid>(hexUUID);
    } catch (...) {
        throw ValueError("HistoryWithContractorCommand::parse: "
                             "Can't parse command. Error occurred while parsing 'Contractor UUID' token.");
    }
    nextTokenSeparatorPos = tokenSeparatorPos + NodeUUID::kHexSize + 1;
    if (nextTokenSeparatorPos + 1 < commandBuffer.length()) {
        throw ValueError("HistoryWithContractorCommand::parse: "
                             "Can't parse command. Command contains extra characters");
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
