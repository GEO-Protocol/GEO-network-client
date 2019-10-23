#include "HistoryTrustLinesCommand.h"

HistoryTrustLinesCommand::HistoryTrustLinesCommand(
    const CommandUUID &uuid,
    const string &commandBuffer):

    BaseUserCommand(
        uuid,
        identifier())
{
    auto check = [&](auto &ctx) {
        if(_attr(ctx) == kCommandsSeparator || _attr(ctx) == kTokensSeparator) {
            throw ValueError("HistoryTrustLinesCommand: input is empty.");
        }
    };
    auto historyFromParse = [&](auto &ctx) {
        mHistoryFrom = _attr(ctx);
    };
    auto historyCountParse = [&](auto &ctx) {
        mHistoryCount = _attr(ctx);
    };
    auto setTimeFromPresentNull = [&](auto &ctx) {
        mIsTimeFromPresent = false;
    };
    auto timeFromPresentAddMicroseconds = [&](auto &ctx) {
        mIsTimeFromPresent = true;
        mTimeFrom = pt::time_from_string("1970-01-01 00:00:00.000");
        mTimeFrom += pt::microseconds(_attr(ctx));
    };
    auto setTimeToPresentNull = [&](auto &ctx) {
        mIsTimeToPresent = false;
    };
    auto timeToPresentAddMicroseconds = [&](auto &ctx) {
        mIsTimeToPresent = true;
        mTimeTo = pt::time_from_string("1970-01-01 00:00:00.000");
        mTimeTo += pt::microseconds(_attr(ctx));
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
                > -(+(char_("null")[setTimeFromPresentNull]))
                > -(ulong_[timeFromPresentAddMicroseconds])
                > char_(kTokensSeparator)
                > -(+(char_("null")[setTimeToPresentNull]))
                > -(ulong_[timeToPresentAddMicroseconds])
                > char_(kTokensSeparator)
                > int_[equivalentParse]
                > eol > eoi));
    } catch(...) {
        throw ValueError("HistoryTrustLinesCommand: cannot parse command.");
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
