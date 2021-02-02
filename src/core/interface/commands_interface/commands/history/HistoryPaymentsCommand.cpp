#include "HistoryPaymentsCommand.h"

HistoryPaymentsCommand::HistoryPaymentsCommand(
    const CommandUUID &uuid,
    const string &commandBuffer):

    BaseUserCommand(
        uuid,
        identifier())
{
    uint32_t flagLow = 0, flagHigh = 0, flag4 = 0, flag8 =0 , flag12 = 0;
    std::string lowBoundaryAmount, highBoundaryAmount, paymentRecordCommandUUID, paymentRecordTransactionUUID;
    auto check = [&](auto &ctx) {
        if(_attr(ctx) == kCommandsSeparator || _attr(ctx) == kTokensSeparator) {
            throw ValueError("HistoryPaymentsCommand: input is empty.");
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
            throw ValueError("HistoryPaymentsCommand: amount contains leading zero.");
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
            throw ValueError("HistoryPaymentsCommand: amount contains leading zero.");
        }
    };
    auto paymentRecordUUIDNull = [&](auto &ctx){
        mIsPaymentRecordCommandUUIDPresent = false;
    };
    auto addUUID8Digits = [&](auto &ctx) {
        flag8++;
        if(flag8 > 9 || (_attr(ctx) == '-' && flag8 < 9)) {
            throw ValueError("HistoryPaymentsCommand: UUID expect 8 digits.");
        }
        mIsPaymentRecordCommandUUIDPresent = true;
        paymentRecordCommandUUID += _attr(ctx);
    };
    auto addUUID4Digits = [&](auto &ctx) {
        flag4++;
        if(flag4 >5 || (_attr(ctx) == '-' && flag4 < 5)) {
            throw ValueError("HistoryPaymentsCommand: UUID expect 4 digits.");
        } else if(_attr(ctx) == '-') {
            flag4 = 0;
        }
        paymentRecordCommandUUID += _attr(ctx);
    };
    auto addUUID12Digits = [&](auto &ctx) {
        flag12++;
        if(flag12 >13 || (_attr(ctx) == kTokensSeparator && flag12 < 13)) {
            throw ValueError("HistoryPaymentsCommand: UUID expect 12 digits.");
        } else if(_attr(ctx) == kTokensSeparator) {
            return;
        } else { paymentRecordCommandUUID += _attr(ctx);}
    };

    auto paymentTransactionUUIDNull = [&](auto &ctx){
        mIsPaymentRecordTransactionUUIDPresent = false;
    };
    auto addTransactionUUID8Digits = [&](auto &ctx) {
        flag8++;
        if(flag8 > 9 || (_attr(ctx) == '-' && flag8 < 9)) {
            throw ValueError("HistoryPaymentsCommand: Transaction UUID expect 8 digits.");
        }
        mIsPaymentRecordTransactionUUIDPresent = true;
        paymentRecordTransactionUUID += _attr(ctx);
    };
    auto addTransactionUUID4Digits = [&](auto &ctx) {
        flag4++;
        if(flag4 >5 || (_attr(ctx) == '-' && flag4 < 5)) {
            throw ValueError("HistoryPaymentsCommand: Transaction UUID expect 4 digits.");
        } else if(_attr(ctx) == '-') {
            flag4 = 0;
        }
        paymentRecordTransactionUUID += _attr(ctx);
    };
    auto addTransactionUUID12Digits = [&](auto &ctx) {
        flag12++;
        if(flag12 >13 || (_attr(ctx) == kTokensSeparator && flag12 < 13)) {
            throw ValueError("HistoryPaymentsCommand: Transaction UUID expect 12 digits.");
        } else if(_attr(ctx) == kTokensSeparator) {
            return;
        } else { paymentRecordTransactionUUID += _attr(ctx);}
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
                >(
                    parserString::string("null")[paymentRecordUUIDNull] |
                    UUIDLexeme<
                        decltype(addTransactionUUID8Digits),
                        decltype(addTransactionUUID4Digits),
                        decltype(addTransactionUUID12Digits)>(
                            addTransactionUUID8Digits,
                            addTransactionUUID4Digits,
                            addTransactionUUID12Digits))
                > char_(kTokensSeparator)
                > *(int_[equivalentParse])
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
        if(mIsPaymentRecordTransactionUUIDPresent){
            mPaymentRecordTransactionUUID = boost::lexical_cast<uuids::uuid>(paymentRecordTransactionUUID);
        }

    } catch(...) {
        throw ValueError("HistoryPaymentsCommand: cannot parse command.");
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

const TransactionUUID& HistoryPaymentsCommand::paymentRecordTransactionUUID() const
{
    return mPaymentRecordTransactionUUID;
}

const bool HistoryPaymentsCommand::isPaymentRecordTransactionUUIDPresent() const
{
    return mIsPaymentRecordTransactionUUIDPresent;
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
