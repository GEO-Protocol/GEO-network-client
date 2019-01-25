#include "HistoryPaymentsCommand.h"

HistoryPaymentsCommand::HistoryPaymentsCommand(
    const CommandUUID &uuid,
    const string &commandBuffer):

    BaseUserCommand(
        uuid,
        identifier())
{
    uint32_t flag_low = 0, flag_high = 0, flag_4 = 0, flag_8 =0 , flag_12 = 0;
    std::string lowBoundaryAmount, highBoundaryAmount, paymentRecordCommandUUID;
    auto check = [&](auto &ctx) { if(_attr(ctx) == '\n'){throw ValueError("HistoryPaymentsCommand: there is no input ");}};
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
        else if (flag_low == 1 && _attr(ctx) <= 0) {throw ValueError("Amount can't be zero or low");}

    };

    auto highboundary_null = [&](auto &ctx) { mIsHighBoundaryAmountPresent = false; };
    auto highboundary_number = [&](auto &ctx)
    {
        highBoundaryAmount += _attr(ctx);
        mIsHighBoundaryAmountPresent = true;
        flag_high++;
        if(flag_high>39) {throw ValueError("Amount is too big");}
        else if (flag_high == 1 && _attr(ctx) <= 0) {throw ValueError("Amount can't be zero or low");}

    };

    auto paymentRecordUUID_null = [&](auto &ctx){ mIsPaymentRecordCommandUUIDPresent = false; };
    auto add_8 = [&](auto &ctx)
    {
        flag_8++;
        if(flag_8 >9){throw 1;}
        else if(_attr(ctx) == '-' && flag_8 < 9) {throw ValueError("Expect 8 digits");}
        mIsPaymentRecordCommandUUIDPresent = true;
        paymentRecordCommandUUID += _attr(ctx);


    };

    auto add_4 = [&](auto &ctx)
    {
        flag_4++;
        if(flag_4 >5 || (_attr(ctx) == '-' && flag_4 < 5)){throw ValueError("Expect 4 digits");}
        else if(_attr(ctx) == '-'){flag_4=0;}
        paymentRecordCommandUUID += _attr(ctx);


    };

    auto add_12 = [&](auto &ctx)
    {
        flag_12++;
        if(flag_12 >13 || (_attr(ctx) == '\t' && flag_12 < 13)){throw ValueError("Expect 12 digits");}
        else if(_attr(ctx) == '\t') {}
        else { paymentRecordCommandUUID += _attr(ctx);}

    };

    auto equivalentID_add = [&](auto &ctx) { mEquivalent = _attr(ctx); };

    try
    {
        parse(commandBuffer.begin(), commandBuffer.end(), char_[check]);
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
                        > -(+(char_("null")[paymentRecordUUID_null] >char_('\t')))
                        > -( *(char_[add_8] - char_('-')) > char_('-') [add_8]
                             >*(char_[add_4] - char_('-')) > char_('-') [add_4]
                             >*(char_[add_4] - char_('-')) > char_('-') [add_4]
                             >*(char_[add_4] - char_('-')) > char_('-') [add_4]
                             >*(char_[add_12] - char_('\t')) > char_('\t') [add_12])
                      > int_[equivalentID_add]
                      > eol

              )
             );

        mLowBoundaryAmount = TrustLineAmount(lowBoundaryAmount);
        mHighBoundaryAmount = TrustLineAmount(highBoundaryAmount);
        mPaymentRecordCommandUUID = boost::lexical_cast<uuids::uuid>(paymentRecordCommandUUID);
    }
    catch(...)
    {
        throw ValueError("HistoryPaymentsCommand : can't parse command");
    }
}

const string &HistoryPaymentsCommand::identifier()
{
    static const string identifier = "GET:history/payments";
    return identifier;
}

const size_t HistoryPaymentsCommand::historyFrom() const
{
    return mHistoryFrom;
}

const size_t HistoryPaymentsCommand::historyCount() const
{
    return mHistoryCount;
}

const DateTime HistoryPaymentsCommand::timeFrom() const
{
    return mTimeFrom;
}

const DateTime HistoryPaymentsCommand::timeTo() const
{
    return mTimeTo;
}

const bool HistoryPaymentsCommand::isTimeFromPresent() const
{
    return mIsTimeFromPresent;
}

const bool HistoryPaymentsCommand::isTimeToPresent() const
{
    return mIsTimeToPresent;
}

const TrustLineAmount& HistoryPaymentsCommand::lowBoundaryAmount() const
{
    return mLowBoundaryAmount;
}

const TrustLineAmount& HistoryPaymentsCommand::highBoundaryAmount() const
{
    return mHighBoundaryAmount;
}

const bool HistoryPaymentsCommand::isLowBoundaryAmountPresent() const
{
    return mIsLowBoundaryAmountPresent;
}

const bool HistoryPaymentsCommand::isHighBoundaryAmountPresent() const
{
    return mIsHighBoundaryAmountPresent;
}

const CommandUUID& HistoryPaymentsCommand::paymentRecordCommandUUID() const
{
    return mPaymentRecordCommandUUID;
}

const bool HistoryPaymentsCommand::isPaymentRecordCommandUUIDPresent() const
{
    return mIsPaymentRecordCommandUUIDPresent;
}

const SerializedEquivalent HistoryPaymentsCommand::equivalent() const
{
    return mEquivalent;
}

CommandResult::SharedConst HistoryPaymentsCommand::resultOk(string &historyPaymentsStr) const
{
    return CommandResult::SharedConst(
        new CommandResult(
            identifier(),
            UUID(),
            200,
            historyPaymentsStr));
}
