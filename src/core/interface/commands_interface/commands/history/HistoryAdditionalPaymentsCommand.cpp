#include "HistoryAdditionalPaymentsCommand.h"

HistoryAdditionalPaymentsCommand::HistoryAdditionalPaymentsCommand(
    const CommandUUID &uuid,
    const string &commandBuffer):

    BaseUserCommand(
        uuid,
        identifier())
{
    uint32_t flagLow = 0, flagHigh = 0;
    std::string lowBoundaryAmount, highBoundaryAmount;
    auto check = [&](auto &ctx) {
        if(_attr(ctx) == kCommandsSeparator || _attr(ctx) == kTokensSeparator) {
            throw ValueError("HistoryAdditionalPayments: input is empty.");
        }
    };
    auto historyFromParse = [&](auto &ctx) {
        mHistoryFrom = _attr(ctx);
    };
    auto historyCountParse = [&](auto &ctx) {
        mHistoryCount = _attr(ctx);
    };
    auto timeFromPresentNull = [&](auto &ctx) {
        mIsTimeFromPresent = false;
    };
    auto timeFromPresentNumber = [&](auto &ctx) {
        mIsTimeFromPresent = true;
        mTimeFrom = pt::time_from_string("1970-01-01 00:00:00.000");
        mTimeFrom += pt::microseconds(_attr(ctx));
    };
    auto timeToPresentNull = [&](auto &ctx) {
        mIsTimeToPresent = false;
    };
    auto timeToPresentNumber = [&](auto &ctx) {
        mIsTimeToPresent = true;
        mTimeTo = pt::time_from_string("1970-01-01 00:00:00.000");
        mTimeTo += pt::microseconds(_attr(ctx));
    };
    auto setLowBoundaryAmountNull = [&](auto &ctx) {
        mIsLowBoundaryAmountPresent = false; };
    auto lowBoundaryAmountNumber = [&](auto &ctx) {
        lowBoundaryAmount += _attr(ctx);
        mIsLowBoundaryAmountPresent = true;
        flagLow++;
        if (flagLow == 1 && _attr(ctx) == '0') {
            throw ValueError("HistoryAdditionalPaymentsCommand: amount contains leading zero.");
        }
    };
    auto setHighBoundaryAmountNull = [&](auto &ctx) {
        mIsHighBoundaryAmountPresent = false;
    };
    auto highBoundaryAmountNumber = [&](auto &ctx) {
        highBoundaryAmount += _attr(ctx);
        mIsHighBoundaryAmountPresent = true;
        flagHigh++;
        if (flagHigh == 1 && _attr(ctx) == '0') {
            throw ValueError("HistoryAdditionalPaymentsCommand: amount contains leading zero.");
        }
    };
    auto equivalentParse = [&](auto &ctx) {
        mEquivalent = _attr(ctx);
    };

    try {
        parse(
            commandBuffer.begin(),
            commandBuffer.end(),
            char_[check]);
        parse(
            commandBuffer.begin(),
            commandBuffer.end(), (
                *(int_[historyFromParse])
                > char_(kTokensSeparator)
                > *(int_[historyCountParse])
                > char_(kTokensSeparator)
                > -(+(char_("null")[timeFromPresentNull]))
                > -(ulong_[timeFromPresentNumber])
                > char_(kTokensSeparator)
                > -(+(char_("null")[timeToPresentNull]))
                > -(ulong_[timeToPresentNumber])
                > char_(kTokensSeparator)
                > -(+(char_("null")[setLowBoundaryAmountNull]))
                > -(*(digit [lowBoundaryAmountNumber] > !alpha > !punct))
                > char_(kTokensSeparator)
                > -(+(char_("null")[setHighBoundaryAmountNull]))
                > -(*(digit [highBoundaryAmountNumber] > !alpha > !punct))
                > char_(kTokensSeparator)
                > int_[equivalentParse]
                > eol > eoi));
        mLowBoundaryAmount = TrustLineAmount(lowBoundaryAmount);
        mHighBoundaryAmount = TrustLineAmount(highBoundaryAmount);
    }
    catch(...) {
       throw ValueError("HistoryAdditionalPaymentsCommand : cannot parse command.");
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
