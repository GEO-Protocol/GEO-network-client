#include "HistoryAdditionalPaymentsCommand.h"

HistoryAdditionalPaymentsCommand::HistoryAdditionalPaymentsCommand(
    const CommandUUID &uuid,
    const string &commandBuffer):

    BaseUserCommand(
        uuid,
        identifier())
{
    const auto minCommandLength = 14;

    if (commandBuffer.size() < minCommandLength) {
        throw ValueError(
                "HistoryAdditionalPaymentsCommand: can't parse command. "
                    "Received command is too short.");
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
                "HistoryAdditionalPaymentsCommand: can't parse command. "
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
                "HistoryAdditionalPaymentsCommand: can't parse command. "
                    "Error occurred while parsing 'count' token.");
    }

    tokenSeparatorPos = nextTokenSeparatorPos;
    nextTokenSeparatorPos = commandBuffer.find(
        kTokensSeparator,
        tokenSeparatorPos + 1);
    string timeFromStr = commandBuffer.substr(
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
            throw ValueError(
                    "HistoryAdditionalPaymentsCommand: can't parse command. "
                        "Error occurred while parsing 'timeFrom' token.");
        }
    }

    tokenSeparatorPos = nextTokenSeparatorPos;
    nextTokenSeparatorPos = commandBuffer.find(
        kTokensSeparator,
        tokenSeparatorPos + 1);
    string timeToStr = commandBuffer.substr(
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
            throw ValueError(
                    "HistoryAdditionalPaymentsCommand: can't parse command. "
                        "Error occurred while parsing 'timeTo' token.");
        }
    }

    tokenSeparatorPos = nextTokenSeparatorPos;
    nextTokenSeparatorPos = commandBuffer.find(
        kTokensSeparator,
        tokenSeparatorPos + 1);
    string lowBoundaryAmountStr = commandBuffer.substr(
        tokenSeparatorPos + 1,
        nextTokenSeparatorPos - tokenSeparatorPos - 1);
    if (lowBoundaryAmountStr == kNullParameter) {
        mIsLowBoundaryAmountPresent = false;
    } else {
        mIsLowBoundaryAmountPresent = true;
        try {
            mLowBoundaryAmount = TrustLineAmount(
                lowBoundaryAmountStr);
        } catch (...) {
            throw ValueError(
                    "HistoryAdditionalPaymentsCommand: can't parse command. "
                        "Error occurred while parsing 'lowBoundaryAmount' token.");
        }
    }

    tokenSeparatorPos = nextTokenSeparatorPos;
    nextTokenSeparatorPos = commandBuffer.find(
        kTokensSeparator,
        tokenSeparatorPos + 1);
    string highBoundaryAmountStr = commandBuffer.substr(
        tokenSeparatorPos + 1,
        nextTokenSeparatorPos - tokenSeparatorPos - 1);

    if (highBoundaryAmountStr == kNullParameter) {
        mIsHighBoundaryAmountPresent = false;
    } else {
        mIsHighBoundaryAmountPresent = true;
        try {
            mHighBoundaryAmount = TrustLineAmount(
                highBoundaryAmountStr);
        } catch (...) {
            throw ValueError(
                    "HistoryAdditionalPaymentsCommand: can't parse command. "
                        "Error occurred while parsing 'mHighBoundaryAmount' token.");
        }
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
                "HistoryAdditionalPaymentsCommand: can't parse command. "
                    "Error occurred while parsing 'equivalent' token.");
    }
}

const string &HistoryAdditionalPaymentsCommand::identifier()
{
    static const string identifier = "GET:history/payments/additional";
    return identifier;
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
    return mIsLowBoundaryAmountPresent;
}

const bool HistoryAdditionalPaymentsCommand::isHighBoundaryAmountPresent() const
{
    return mIsHighBoundaryAmountPresent;
}

const SerializedEquivalent HistoryAdditionalPaymentsCommand::equivalent() const
{
    return mEquivalent;
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
