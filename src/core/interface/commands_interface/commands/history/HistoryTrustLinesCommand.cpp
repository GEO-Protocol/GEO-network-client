#include "HistoryTrustLinesCommand.h"

HistoryTrustLinesCommand::HistoryTrustLinesCommand(
        const CommandUUID &uuid,
        const string &commandBuffer):

        BaseUserCommand(
                uuid,
                identifier())
{
    parse(commandBuffer);
}

const string &HistoryTrustLinesCommand::identifier()
{
    static const string identifier = "GET:history/trust-lines";
    return identifier;
}

/**
 * Throws ValueError if deserialization was unsuccessful.
 */
void HistoryTrustLinesCommand::parse(
        const string &command)
{
    const auto minCommandLength = 13;
    if (command.size() < minCommandLength) {
        throw ValueError("HistoryTrustLinesCommand::parse: "
                                 "Can't parse command. Received command is to short.");
    }
    size_t tokenSeparatorPos = command.find(
        kTokensSeparator);
    string historyFromStr = command.substr(
        0,
        tokenSeparatorPos);
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

    size_t nextTokenSeparatorPos = command.find(
        kTokensSeparator,
        tokenSeparatorPos + 1);
    string historyCountStr = command.substr(
        tokenSeparatorPos + 1,
        nextTokenSeparatorPos - tokenSeparatorPos - 1);
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
    tokenSeparatorPos = nextTokenSeparatorPos;
    nextTokenSeparatorPos = command.find(
        kTokensSeparator,
        tokenSeparatorPos + 1);
    string timeFromStr = command.substr(
        tokenSeparatorPos + 1,
        nextTokenSeparatorPos - tokenSeparatorPos - 1);
    if (timeFromStr == kNullParameter) {
        mIsTimeFromPresent = false;
    } else {
        mIsTimeFromPresent = true;
        try {
            int64_t timeFrom = std::stoul(timeFromStr);
            mTimeFrom = pt::time_from_string("1970-01-01 00:00:00.000");
            mTimeFrom += pt::microseconds(timeFrom);
        } catch (...) {
            throw ValueError("HistoryTrustLinesCommand::parse: "
                                 "Can't parse command. Error occurred while parsing 'timeFrom' token.");
        }
    }

    tokenSeparatorPos = nextTokenSeparatorPos;
    nextTokenSeparatorPos = command.size() - 1;
    string timeToStr = command.substr(
        tokenSeparatorPos + 1,
        nextTokenSeparatorPos - tokenSeparatorPos - 1);
    if (timeToStr == kNullParameter) {
        mIsTimeToPresent = false;
    } else {
        mIsTimeToPresent = true;
        try {
            int64_t timeTo = std::stoul(timeToStr);
            mTimeTo = pt::time_from_string("1970-01-01 00:00:00.000");
            mTimeTo += pt::microseconds(timeTo);
        } catch (...) {
            throw ValueError("HistoryTrustLinesCommand::parse: "
                                 "Can't parse command. Error occurred while parsing 'timeTo' token.");
        }
    }
}

const size_t HistoryTrustLinesCommand::historyFrom() const
{
    return mHistoryFrom;
}

const size_t HistoryTrustLinesCommand::historyCount() const
{
    return mHistoryCount;
}

const DateTime HistoryTrustLinesCommand::timeFrom() const
{
    return mTimeFrom;
}

const DateTime HistoryTrustLinesCommand::timeTo() const
{
    return mTimeTo;
}

const bool HistoryTrustLinesCommand::isTimeFromPresent() const
{
    return mIsTimeFromPresent;
}

const bool HistoryTrustLinesCommand::isTimeToPresent() const
{
    return mIsTimeToPresent;
}

CommandResult::SharedConst HistoryTrustLinesCommand::resultOk(string &historyTrustLinesStr) const
{
    return CommandResult::SharedConst(
        new CommandResult(
            identifier(),
            UUID(),
            200,
            historyTrustLinesStr));
}
