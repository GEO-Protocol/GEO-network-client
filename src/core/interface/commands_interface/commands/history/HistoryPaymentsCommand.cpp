#include "HistoryPaymentsCommand.h"

HistoryPaymentsCommand::HistoryPaymentsCommand(
    const CommandUUID &uuid,
    const string &commandBuffer):

    BaseUserCommand(
        uuid,
        identifier())
{
    uint32_t flagLow = 0, flagHigh = 0, flag4 = 0, flag8 =0 , flag12 = 0;
    std::string lowBoundaryAmount, highBoundaryAmount, paymentRecordCommandUUID;
    auto check = [&](auto &ctx) {
        if(_attr(ctx) == kCommandsSeparator) {
            throw ValueError("HistoryPaymentsCommand: there is no input ");
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
        mIsLowBoundaryAmountPresent = false;
    };
    auto lowBoundaryAmountAddNumber = [&](auto &ctx) {
        lowBoundaryAmount += _attr(ctx);
        mIsLowBoundaryAmountPresent = true;
        flagLow++;
        if(flagLow>39) {
            throw ValueError("Amount is too big");
        } else if (flagLow == 1 && _attr(ctx) <= 0) {
            throw ValueError("Amount can't be zero or low");
        }
    };
    auto highBoundaryAmountNull = [&](auto &ctx) {
        mIsHighBoundaryAmountPresent = false;
    };
    auto highBoundaryAmountAddNumber = [&](auto &ctx) {
        highBoundaryAmount += _attr(ctx);
        mIsHighBoundaryAmountPresent = true;
        flagHigh++;
        if(flagHigh>39) {
            throw ValueError("Amount is too big");
        } else if (flagHigh == 1 && _attr(ctx) <= 0) {
            throw ValueError("Amount can't be zero or low");
        }
    };
    auto paymentRecordUUIDNull = [&](auto &ctx){
        mIsPaymentRecordCommandUUIDPresent = false;
    };
    auto addUUID8Digits = [&](auto &ctx) {
        flag8++;
        if(flag8 >9) {
            throw 1;
        } else if(_attr(ctx) == '-' && flag8 < 9) {
            throw ValueError("Expect 8 digits");
        }
        mIsPaymentRecordCommandUUIDPresent = true;
        paymentRecordCommandUUID += _attr(ctx);
    };
    auto addUUID4Digits = [&](auto &ctx) {
        flag4++;
        if(flag4 >5 || (_attr(ctx) == '-' && flag4 < 5)) {
            throw ValueError("Expect 4 digits");
        } else if(_attr(ctx) == '-') {
            flag4 = 0;
        }
        paymentRecordCommandUUID += _attr(ctx);
    };
    auto addUUID12Digits = [&](auto &ctx) {
        flag12++;
        if(flag12 >13 || (_attr(ctx) == kTokensSeparator && flag12 < 13)) {
            throw ValueError("Expect 12 digits");
        } else if(_attr(ctx) == kTokensSeparator) {
        } else { paymentRecordCommandUUID += _attr(ctx);
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
                > -(*(digit [lowBoundaryAmountAddNumber] > !alpha > !punct))
                > char_(kTokensSeparator)
                > -(+(char_("null")[highBoundaryAmountNull]))
                > -(*(digit [highBoundaryAmountAddNumber] > !alpha > !punct))
                > char_(kTokensSeparator)
                    > -(+(char_("null")[paymentRecordUUIDNull] >char_('\t')))
                    > -( *(char_[addUUID8Digits] - char_('-')) > char_('-') [addUUID8Digits]
                        >*(char_[addUUID4Digits] - char_('-')) > char_('-') [addUUID4Digits]
                        >*(char_[addUUID4Digits] - char_('-')) > char_('-') [addUUID4Digits]
                        >*(char_[addUUID4Digits] - char_('-')) > char_('-') [addUUID4Digits]
                        >*(char_[addUUID12Digits] - char_('\t')) > char_('\t') [addUUID12Digits])
                > int_[equivalentParse]
                > eol));

        mLowBoundaryAmount = TrustLineAmount(lowBoundaryAmount);
        mHighBoundaryAmount = TrustLineAmount(highBoundaryAmount);
        mPaymentRecordCommandUUID = boost::lexical_cast<uuids::uuid>(paymentRecordCommandUUID);
    } catch(...) {
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
