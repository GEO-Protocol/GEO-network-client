#include "HistoryAdditionalPaymentsCommand.h"

HistoryAdditionalPaymentsCommand::HistoryAdditionalPaymentsCommand(
    const CommandUUID &uuid,
    const string &commandBuffer):

    BaseUserCommand(
        uuid,
        identifier())
{
    parse(commandBuffer);
}

const string &HistoryAdditionalPaymentsCommand::identifier()
{
    static const string identifier = "GET:history/payments/additional";
    return identifier;
}

/**
 * Throws ValueError if deserialization was unsuccessful.
 */
void HistoryAdditionalPaymentsCommand::parse(
    const string &command)
{
    const auto minCommandLength = 28;

    cout << "######################" << endl;
    cout << command.size() << endl;

    if (command.size() < minCommandLength) {
        throw ValueError("HistoryAdditionalPaymentsCommand::parse: "
                             "Can't parse command. Received command is too short.");
    }
    size_t tokenSeparatorPos = command.find(
        kTokensSeparator);
    string historyFromStr = command.substr(
        0,
        tokenSeparatorPos);
    if (historyFromStr.at(0) == '-') {
        throw ValueError("HistoryAdditionalPaymentsCommand::parse: "
                             "Can't parse command. 'from' token can't be negative.");
    }
    try {
        mHistoryFrom = std::stoul(historyFromStr);
    } catch (...) {
        throw ValueError("HistoryAdditionalPaymentsCommand::parse: "
                             "Can't parse command. Error occurred while parsing  'from' token.");
    }
    cout << "historyFromStr: " << historyFromStr << endl;
    cout << "tokenSeparatorPos: " << tokenSeparatorPos << endl;

    size_t nextTokenSeparatorPos = command.find(
        kTokensSeparator,
        tokenSeparatorPos + 1);
    string historyCountStr = command.substr(
        tokenSeparatorPos + 1,
        nextTokenSeparatorPos - tokenSeparatorPos - 1);
    if (historyCountStr.at(0) == '-') {
        throw ValueError("HistoryAdditionalPaymentsCommand::parse: "
                             "Can't parse command. 'count' token can't be negative.");
    }
    try {
        mHistoryCount = std::stoul(historyCountStr);
    } catch (...) {
        throw ValueError("HistoryAdditionalPaymentsCommand::parse: "
                             "Can't parse command. Error occurred while parsing 'count' token.");
    }

    cout << "historyCountStr: " << historyCountStr << endl;
    cout << "tokenSeparatorPos: " << tokenSeparatorPos << endl;
    cout << "nextTokenSeparatorPos: " << nextTokenSeparatorPos << endl;

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
            throw ValueError("HistoryAdditionalPaymentsCommand::parse: "
                                 "Can't parse command. Error occurred while parsing 'timeFrom' token.");
        }
    }

    cout << "timeFromStr: " << timeFromStr << endl;
    cout << "tokenSeparatorPos: " << tokenSeparatorPos << endl;
    cout << "nextTokenSeparatorPos: " << nextTokenSeparatorPos << endl;

    tokenSeparatorPos = nextTokenSeparatorPos;
    nextTokenSeparatorPos = command.find(
        kTokensSeparator,
        tokenSeparatorPos + 1);
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
            throw ValueError("HistoryAdditionalPaymentsCommand::parse: "
                                 "Can't parse command. Error occurred while parsing 'timeTo' token.");
        }
    }

    cout << "timeToStr: " << timeToStr << endl;
    cout << "tokenSeparatorPos: " << tokenSeparatorPos << endl;
    cout << "nextTokenSeparatorPos: " << nextTokenSeparatorPos << endl;

    tokenSeparatorPos = nextTokenSeparatorPos;
    nextTokenSeparatorPos = command.find(
        kTokensSeparator,
        tokenSeparatorPos + 1);
    string lowBoundaryAmountStr = command.substr(
        tokenSeparatorPos + 1,
        nextTokenSeparatorPos - tokenSeparatorPos - 1);
    if (lowBoundaryAmountStr == kNullParameter) {
        mIsLowBoundartAmountPresent = false;
    } else {
        mIsLowBoundartAmountPresent = true;
        try {
            mLowBoundaryAmount = TrustLineAmount(
                lowBoundaryAmountStr);
        } catch (...) {
            throw ValueError("HistoryAdditionalPaymentsCommand::parse: "
                                 "Can't parse command. Error occurred while parsing 'lowBoundaryAmount' token.");
        }
    }

    cout << "lowBoundaryAmountStr: " << lowBoundaryAmountStr << endl;
    cout << "tokenSeparatorPos: " << tokenSeparatorPos << endl;
    cout << "nextTokenSeparatorPos: " << nextTokenSeparatorPos << endl;

    tokenSeparatorPos = nextTokenSeparatorPos;
    nextTokenSeparatorPos = command.find(
        kTokensSeparator,
        tokenSeparatorPos + 1);
    string highBoundaryAmountStr = command.substr(
        tokenSeparatorPos + 1,
        nextTokenSeparatorPos - tokenSeparatorPos - 1);

    cout << "highBoundaryAmountStr: " << highBoundaryAmountStr << endl;
    cout << "tokenSeparatorPos: " << tokenSeparatorPos << endl;
    cout << "nextTokenSeparatorPos: " << nextTokenSeparatorPos << endl;

    if (highBoundaryAmountStr == kNullParameter) {
        mIsHighBoundaryAmountPresent = false;
    } else {
        mIsHighBoundaryAmountPresent = true;
        try {
            mHighBoundaryAmount = TrustLineAmount(
                highBoundaryAmountStr);
        } catch (...) {
            throw ValueError("HistoryAdditionalPaymentsCommand::parse: "
                                 "Can't parse command. Error occurred while parsing 'mHighBoundaryAmount' token.");
        }
    }
}

const size_t HistoryAdditionalPaymentsCommand::historyFrom() const
{
    return mHistoryFrom;
}

const size_t HistoryAdditionalPaymentsCommand::historyCount() const
{
    return mHistoryCount;
}

const DateTime HistoryAdditionalPaymentsCommand::timeFrom() const
{
    return mTimeFrom;
}

const DateTime HistoryAdditionalPaymentsCommand::timeTo() const
{
    return mTimeTo;
}

const bool HistoryAdditionalPaymentsCommand::isTimeFromPresent() const
{
    return mIsTimeFromPresent;
}

const bool HistoryAdditionalPaymentsCommand::isTimeToPresent() const
{
    return mIsTimeToPresent;
}

const TrustLineAmount& HistoryAdditionalPaymentsCommand::lowBoundaryAmount() const
{
    return mLowBoundaryAmount;
}

const TrustLineAmount& HistoryAdditionalPaymentsCommand::highBoundaryAmount() const
{
    return mHighBoundaryAmount;
}

const bool HistoryAdditionalPaymentsCommand::isLowBoundaryAmountPresent() const
{
    return mIsLowBoundartAmountPresent;
}

const bool HistoryAdditionalPaymentsCommand::isHighBoundaryAmountPresent() const
{
    return mIsHighBoundaryAmountPresent;
}

CommandResult::SharedConst HistoryAdditionalPaymentsCommand::resultOk(string &historyPaymentsStr) const
{
    return CommandResult::SharedConst(
        new CommandResult(
            identifier(),
            UUID(),
            200,
            historyPaymentsStr));
}
