#include "HistoryTrustLinesCommand.h"

HistoryTrustLinesCommand::HistoryTrustLinesCommand(
    const CommandUUID &uuid,
    const string &commandBuffer):

    BaseUserCommand(
        uuid,
        identifier())
{
    auto check = [&](auto &ctx) { if(_attr(ctx) == '\n'){throw ValueError("HistoryTrustLinesCommand: there is no input ");}};
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
                      > int_[equivalentID_add]
                      > eol

              )
             );

    }
    catch(...)
    {
        throw ValueError("HistoryTrustLinesCommand : can't parse command");
    }
}

const string &HistoryTrustLinesCommand::identifier()
{
    static const string identifier = "GET:history/trust-lines";
    return identifier;
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

const SerializedEquivalent HistoryTrustLinesCommand::equivalent() const
{
    return mEquivalent;
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
