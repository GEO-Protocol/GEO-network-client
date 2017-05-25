#include "HistoryPaymentsCommand.h"

HistoryPaymentsCommand::HistoryPaymentsCommand(
    const CommandUUID &uuid,
    const string &commandBuffer):

    BaseUserCommand(
        uuid,
        identifier())
{
    parse(commandBuffer);
}

const string &HistoryPaymentsCommand::identifier()
{
    static const string identifier = "GET:history/payments";
    return identifier;
}

/**
 * Throws ValueError if deserialization was unsuccessful.
 */
void HistoryPaymentsCommand::parse(
        const string &command)
{
    const auto minCommandLength = 3;
    if (command.size() < minCommandLength) {
        throw ValueError("HistoryPaymentsCommand::parse: "
                                 "Can't parse command. Received command is to short.");
    }
    size_t tokenSeparatorPos = command.find(kTokensSeparator);
    string historyFromStr = command.substr(
        0,
        tokenSeparatorPos);
    if (historyFromStr.at(0) == '-') {
        throw ValueError("HistoryPaymentsCommand::parse: "
                                 "Can't parse command. 'from' token can't be negative.");
    }
    try {
        mHistoryFrom = std::stoul(historyFromStr);
    } catch (...) {
        throw ValueError("HistoryPaymentsCommand::parse: "
                                 "Can't parse command. Error occurred while parsing  'from' token.");
    }
    string historyCountStr = command.substr(
        tokenSeparatorPos + 1,
        command.size() - 1);
    if (historyCountStr.at(0) == '-') {
        throw ValueError("HistoryPaymentsCommand::parse: "
                                 "Can't parse command. 'count' token can't be negative.");
    }
    try {
        mHistoryCount = std::stoul(historyCountStr);
    } catch (...) {
        throw ValueError("HistoryPaymentsCommand::parse: "
                                 "Can't parse command. Error occurred while parsing 'count' token.");
    }
}

const size_t HistoryPaymentsCommand::historyFrom() const
{
    return mHistoryFrom;
}

const size_t HistoryPaymentsCommand::historyCount() const
{
    return mHistoryCount;
}

CommandResult::SharedConst HistoryPaymentsCommand::resultOk(string &historyPaymentsStr) const
{
    return CommandResult::SharedConst(
        new CommandResult(
            UUID(),
            200,
            historyPaymentsStr));
}
