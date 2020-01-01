#include "HistoryPaymentsAllEquivalentsCommand.h"

HistoryPaymentsAllEquivalentsCommand::HistoryPaymentsAllEquivalentsCommand(
    const CommandUUID &uuid,
    const string &commandBuffer):

    BaseUserCommand(
        uuid,
        identifier())
{
    uint32_t flagLow = 0, flagHigh = 0, flag4 = 0, flag8 =0 , flag12 = 0;
    std::string lowBoundaryAmount, highBoundaryAmount, paymentRecordCommandUUID;
    auto check = [&](auto &ctx) {
        if(_attr(ctx) == kCommandsSeparator || _attr(ctx) == kTokensSeparator) {
            throw ValueError("HistoryPaymentsAllEquivalentsCommand: input is empty.");
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
        mIsLowBoundaryAmountPresent = false;
    };
    auto lowBoundaryAmountAddNumber = [&](auto &ctx) {
        lowBoundaryAmount += _attr(ctx);
        mIsLowBoundaryAmountPresent = true;
        flagLow++;
        if (flagLow > 1 && lowBoundaryAmount[0] == '0') {
            throw ValueError("HistoryPaymentsAllEquivalentsCommand: amount contains leading zero.");
        }
    };
    auto setHighBoundaryAmountNull = [&](auto &ctx) {
        mIsHighBoundaryAmountPresent = false;
    };
    auto highBoundaryAmountAddNumber = [&](auto &ctx) {
        highBoundaryAmount += _attr(ctx);
        mIsHighBoundaryAmountPresent = true;
        flagHigh++;
        if (flagHigh > 1 && highBoundaryAmount[0] == '0') {
            throw ValueError("HistoryPaymentsAllEquivalentsCommand: amount contains leading zero.");
        }
    };
    auto paymentRecordUUIDNull = [&](auto &ctx){
        mIsPaymentRecordCommandUUIDPresent = false;
    };
    auto addUUID8Digits = [&](auto &ctx) {
        flag8++;
        if(flag8 > 9 || (_attr(ctx) == '-' && flag8 < 9)) {
            throw ValueError("HistoryPaymentsAllEquivalentsCommand: UUID expect 8 digits.");
        }
        mIsPaymentRecordCommandUUIDPresent = true;
        paymentRecordCommandUUID += _attr(ctx);
    };
    auto addUUID4Digits = [&](auto &ctx) {
        flag4++;
        if(flag4 >5 || (_attr(ctx) == '-' && flag4 < 5)) {
            throw ValueError("HistoryPaymentsAllEquivalentsCommand: UUID expect 4 digits.");
        } else if(_attr(ctx) == '-') {
            flag4 = 0;
        }
        paymentRecordCommandUUID += _attr(ctx);
    };
    auto addUUID12Digits = [&](auto &ctx) {
        flag12++;
        if(flag12 >13 || (_attr(ctx) == kTokensSeparator && flag12 < 13)) {
            throw ValueError("HistoryPaymentsAllEquivalentsCommand: UUID expect 12 digits.");
        } else if(_attr(ctx) == kTokensSeparator) {
            return;
        } else { paymentRecordCommandUUID += _attr(ctx);}
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
                > (
                    (parserString::string("null")[timeFromPresentNull]) |
                    *(ulong_[timeFromPresentNumber]))
                > char_(kTokensSeparator)
                > (
                    (parserString::string("null")[timeToPresentNull]) |
                    *(ulong_[timeToPresentNumber]))
                > char_(kTokensSeparator)
                >(
                    (parserString::string("null")[setLowBoundaryAmountNull]) |
                    *(digit [lowBoundaryAmountAddNumber] > !alpha > !punct))
                > char_(kTokensSeparator)
                >(
                    (parserString::string("null")[setHighBoundaryAmountNull]) |
                    *(digit [highBoundaryAmountAddNumber] > !alpha > !punct ))
                > char_(kTokensSeparator)
                >(
                    parserString::string("null")[paymentRecordUUIDNull] |
                    UUIDLexeme<
                        decltype(addUUID8Digits),
                        decltype(addUUID4Digits),
                        decltype(addUUID12Digits)>(
                            addUUID8Digits,
                            addUUID4Digits,
                            addUUID12Digits))
                > eol > eoi));

        if(mIsLowBoundaryAmountPresent){
            mLowBoundaryAmount = TrustLineAmount(lowBoundaryAmount);
        }
        if(mIsHighBoundaryAmountPresent){
            mHighBoundaryAmount = TrustLineAmount(highBoundaryAmount);
        }
        if(mIsPaymentRecordCommandUUIDPresent){
            mPaymentRecordCommandUUID = boost::lexical_cast<uuids::uuid>(paymentRecordCommandUUID);
        }

    } catch(...) {
        throw ValueError("HistoryPaymentsAllEquivalentsCommand: cannot parse command.");
    }
}

const string &HistoryPaymentsAllEquivalentsCommand::identifier()
{
    static const string identifier = "GET:history/payments/all";
    return identifier;
}

const size_t HistoryPaymentsAllEquivalentsCommand::historyFrom() const
{
    return mHistoryFrom;
}

const size_t HistoryPaymentsAllEquivalentsCommand::historyCount() const
{
    return mHistoryCount;
}

const DateTime HistoryPaymentsAllEquivalentsCommand::timeFrom() const
{
    return mTimeFrom;
}

const DateTime HistoryPaymentsAllEquivalentsCommand::timeTo() const
{
    return mTimeTo;
}

const bool HistoryPaymentsAllEquivalentsCommand::isTimeFromPresent() const
{
    return mIsTimeFromPresent;
}

const bool HistoryPaymentsAllEquivalentsCommand::isTimeToPresent() const
{
    return mIsTimeToPresent;
}

const TrustLineAmount& HistoryPaymentsAllEquivalentsCommand::lowBoundaryAmount() const
{
    return mLowBoundaryAmount;
}

const TrustLineAmount& HistoryPaymentsAllEquivalentsCommand::highBoundaryAmount() const
{
    return mHighBoundaryAmount;
}

const bool HistoryPaymentsAllEquivalentsCommand::isLowBoundaryAmountPresent() const
{
    return mIsLowBoundaryAmountPresent;
}

const bool HistoryPaymentsAllEquivalentsCommand::isHighBoundaryAmountPresent() const
{
    return mIsHighBoundaryAmountPresent;
}

const CommandUUID& HistoryPaymentsAllEquivalentsCommand::paymentRecordCommandUUID() const
{
    return mPaymentRecordCommandUUID;
}

const bool HistoryPaymentsAllEquivalentsCommand::isPaymentRecordCommandUUIDPresent() const
{
    return mIsPaymentRecordCommandUUIDPresent;
}

CommandResult::SharedConst HistoryPaymentsAllEquivalentsCommand::resultOk(string &historyPaymentsStr) const
{
    return CommandResult::SharedConst(
        new CommandResult(
            identifier(),
            UUID(),
            200,
            historyPaymentsStr));
}