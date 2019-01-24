#include "HistoryAdditionalPaymentsCommand.h"

HistoryAdditionalPaymentsCommand::HistoryAdditionalPaymentsCommand(
    const CommandUUID &uuid,
    const string &commandBuffer):

    BaseUserCommand(
        uuid,
        identifier())
{


    uint32_t flag_low = 0, flag_high = 0;
    std::string lowBoundaryAmount, highBoundaryAmount;
    auto historyfrom_add = [&](auto &ctx) { mHistoryFrom = _attr(ctx); };
    auto historycount_add = [&](auto &ctx) { mHistoryCount = _attr(ctx); };
    auto timefrompresent_null = [&](auto &ctx) { mIsTimeFromPresent = false; };
    auto timefrompresent_number = [&](auto &ctx)
    {
        mIsTimeFromPresent = true;
        mTimeFrom = pt::time_from_string("1970-01-01 00:00:00.000");
        mTimeFrom += pt::microseconds(_attr(ctx));

    };

    auto timetopresent_null = [&](auto &ctx) { mIsTimeToPresent = false; };
    auto timetopresent_number = [&](auto &ctx)
    {
        mIsTimeToPresent = true;
        mTimeTo = pt::time_from_string("1970-01-01 00:00:00.000");
        mTimeTo += pt::microseconds(_attr(ctx));

    };

    auto lowboundary_null = [&](auto &ctx) { mIsLowBoundaryAmountPresent = false; };
    auto lowboundary_number = [&](auto &ctx)
    {
        lowBoundaryAmount += _attr(ctx);
        mIsLowBoundaryAmountPresent = true;
        flag_low++;
        if(flag_low>39) {throw ValueError("Amount is too big");}

    };

    auto highboundary_null = [&](auto &ctx) { mIsHighBoundaryAmountPresent = false; };
    auto highboundary_number = [&](auto &ctx)
    {
        highBoundaryAmount += _attr(ctx);
        mIsHighBoundaryAmountPresent = true;
        flag_high++;
        if(flag_high>39) {throw ValueError("Amount is too big");}


    };

    auto equivalentID_add = [&](auto &ctx) { mEquivalent = _attr(ctx); };

    try
    {
    parse(commandBuffer.begin(), commandBuffer.end(),
          (
                  *(int_[historyfrom_add])
                  > char_('\t')
                  > *(int_[historycount_add])
                  > char_('\t')
                  > -(+(char_("null")[timefrompresent_null]))
                  > -(int_[timefrompresent_number])
                  > char_('\t')
                  > -(+(char_("null")[timetopresent_null]))
                  > -(int_[timetopresent_number])
                  > char_('\t')
                  > -(+(char_("null")[lowboundary_null]))
                  > -(*(digit [lowboundary_number] > !alpha > !punct))
                  > char_('\t')
                  > -(+(char_("null")[highboundary_null]))
                  > -(*(digit [highboundary_number] > !alpha > !punct))
                  > char_('\t')
                  > int_[equivalentID_add]
                  > eol

          )
    );

    mLowBoundaryAmount = TrustLineAmount(lowBoundaryAmount);
    mHighBoundaryAmount = TrustLineAmount(highBoundaryAmount);

    }
    catch(...)
    {
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
