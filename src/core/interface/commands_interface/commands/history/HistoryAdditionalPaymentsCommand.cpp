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
            throw ValueError("HistoryAdditionalPayments: there is no input ");
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
    auto lowBoundaryAmountNull = [&](auto &ctx) {
        mIsLowBoundaryAmountPresent = false; };
    auto lowBoundaryAmountNumber = [&](auto &ctx) {
        if(lowBoundaryAmount.front() == '0') {throw ValueError("Amount start's from zero");}
        lowBoundaryAmount += _attr(ctx);
        mIsLowBoundaryAmountPresent = true;
        flagLow++;
        if(flagLow >= 78) {
            for(int i = 0 ; i < lowBoundaryAmount.length(); i++)
            {
                if(lowBoundaryAmount[i] != kAmountLimit[i])
                {
                    throw ValueError("Amount is too big");
                }

            }
        }else if (flagLow == 1 && _attr(ctx) == '0') {
            throw ValueError("Amount can't be zero or low");
        }
    };
    auto highBoundaryAmountNull = [&](auto &ctx) {
        mIsHighBoundaryAmountPresent = false;
    };
    auto highBoundaryAmountNumber = [&](auto &ctx) {
        if(highBoundaryAmount.front() == '0') {throw ValueError("Amount start's from zero");}
        highBoundaryAmount += _attr(ctx);
        mIsHighBoundaryAmountPresent = true;
        flagHigh++;
        if(flagHigh >= 78) {
            for(int i = 0 ; i < highBoundaryAmount.length(); i++)
            {
                if(highBoundaryAmount[i] != kAmountLimit[i])
                {
                    throw ValueError("Amount is too big");
                }

            }
        }else if (flagHigh == 1 && _attr(ctx) == '0') {
            throw ValueError("Amount can't be zero or low");
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
                > -(int_[timeFromPresentNumber])
                > char_(kTokensSeparator)
                > -(+(char_("null")[timeToPresentNull]))
                > -(int_[timeToPresentNumber])
                > char_(kTokensSeparator)
                > -(+(char_("null")[lowBoundaryAmountNull]))
                > -(*(digit [lowBoundaryAmountNumber] > !alpha > !punct))
                > char_(kTokensSeparator)
                > -(+(char_("null")[highBoundaryAmountNull]))
                > -(*(digit [highBoundaryAmountNumber] > !alpha > !punct))
                > char_(kTokensSeparator)
                > int_[equivalentParse]
                > eol > eoi));
        mLowBoundaryAmount = TrustLineAmount(lowBoundaryAmount);
        mHighBoundaryAmount = TrustLineAmount(highBoundaryAmount);
    }
    catch(...) {
       throw ValueError("HistoryAdditionalPaymentsCommand : can't parse command");
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
