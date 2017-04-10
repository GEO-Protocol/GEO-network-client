#include "HistoryTrustLinesCommand.h"

HistoryTrustLinesCommand::HistoryTrustLinesCommand(
        const CommandUUID &uuid,
        const string &commandBuffer):

        BaseUserCommand(
                uuid,
                identifier()) {

    parse(commandBuffer);
}

const string &HistoryTrustLinesCommand::identifier() {

    static const string identifier = "GET:/history/trust-lines";
    return identifier;
}

/**
 * Throws ValueError if deserialization was unsuccessful.
 */
void HistoryTrustLinesCommand::parse(
        const string &command) {

    const auto minCommandLength = 3;

    if (command.size() < minCommandLength) {
        throw ValueError("HistoryTrustLinesCommand::parse: "
                                 "Can't parse command. Received command is to short.");
    }

    size_t tabSeparator = command.find("\t");
    string historyFromStr = command.substr(
        0,
        tabSeparator);
    if (historyFromStr.at(0) == '-') {
        throw ValueError("HistoryTrustLinesCommand::parse: "
                                  "Can't parse command. 'from' token can't be negative.");
    }
    try {
        mHistoryFrom = std::stoul(historyFromStr);
    } catch (...) {
        throw ValueError("HistoryTrustLinesCommand::parse: "
                                 "Can't parse command. Error occurred while parsing  'from' token.");
    }

    string historyCountStr = command.substr(
        tabSeparator + 1,
        command.size() - 1);
    if (historyCountStr.at(0) == '-') {
        throw ValueError("HistoryTrustLinesCommand::parse: "
                                 "Can't parse command. 'count' token can't be negative.");
    }
    try {
        mHistoryCount = std::stoul(historyCountStr);
    } catch (...) {
        throw ValueError("HistoryTrustLinesCommand::parse: "
                                 "Can't parse command. Error occurred while parsing 'count' token.");
    }
}

const size_t HistoryTrustLinesCommand::historyFrom() const {

    return mHistoryFrom;
}

const size_t HistoryTrustLinesCommand::historyCount() const {

    return mHistoryCount;
}

CommandResult::SharedConst HistoryTrustLinesCommand::resultOk(string &historyTrustLinesStr) const {

    return CommandResult::SharedConst(
        new CommandResult(
            UUID(),
            200,
            historyTrustLinesStr));
}
