#include "HistoryPaymentsCommand.h"

HistoryPaymentsCommand::HistoryPaymentsCommand(
    const CommandUUID &uuid,
    const string &commandBuffer):

    BaseUserCommand(
        uuid,
        identifier()) {

    parse(commandBuffer);
}

const string &HistoryPaymentsCommand::identifier() {

    static const string identifier = "GET:/history/payments";
    return identifier;
}

/**
 * Throws ValueError if deserialization was unsuccessful.
 */
void HistoryPaymentsCommand::parse(
        const string &command) {

    const auto minCommandLength = 3;

    if (command.size() < minCommandLength) {
        throw ValueError("HistoryPaymentsCommand::parse: "
                                 "Can't parse command. Received command is to short.");
    }

    try {
        size_t tabSeparator = command.find("\t");
        string historyFromStr = command.substr(
                0,
                tabSeparator);
        mHistoryFrom = std::stoul(historyFromStr);

        string historyCountStr = command.substr(
                tabSeparator + 1,
                command.size() - 1);
        mHistoryCount = std::stoul(historyCountStr);

    } catch (...) {
        throw ValueError("HistoryPaymentsCommand::parse: "
             "Can't parse command. Error occurred while parsing 'count and from' token.");
    }

}

const size_t HistoryPaymentsCommand::historyFrom() const {

    return mHistoryFrom;
}

const size_t HistoryPaymentsCommand::historyCount() const {

    return mHistoryCount;
}

CommandResult::SharedConst HistoryPaymentsCommand::resultOk(string &historyPaymentsStr) const {

    return CommandResult::SharedConst(
        new CommandResult(
            UUID(),
            200,
            historyPaymentsStr));
}
