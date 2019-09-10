#include "ResetTrustLineCommand.h"

ResetTrustLineCommand::ResetTrustLineCommand(
    const CommandUUID &commandUUID,
    const string &command) :

    BaseUserCommand(
        commandUUID,
        identifier())
{
    std::string incomingAmount, outgoingAmount, balance;
    uint32_t flagIncomingAmount=0, flagOutgoingAmount=0, flagBalance=0;
    bool isBalanceNegative=false;
    auto check = [&](auto &ctx) {
        if(_attr(ctx) == kCommandsSeparator || _attr(ctx) == kTokensSeparator) {
            throw ValueError("ResetTrustLineCommand: input is empty.");
        }
    };
    auto contractorIDParse = [&](auto &ctx) {
        mContractorID = _attr(ctx);
    };
    auto auditNumberParse = [&](auto &ctx) {
        mAuditNumber = _attr(ctx);
    };
    auto incomingAmountAddNumber = [&](auto &ctx) {
        if(incomingAmount.front() == '0' && isdigit(incomingAmount.back())){
            throw ValueError("ResetTrustLineCommand: incomingAmount contains leading zero.");
        }
        incomingAmount += _attr(ctx);
        flagIncomingAmount++;
        if (flagIncomingAmount > 1 && incomingAmount.front() == '0') {
            throw ValueError("ResetTrustLineCommand: incomingAmount contains leading zero.");
        }
    };
    auto outgoingAmountAddNumber = [&](auto &ctx) {
        if(outgoingAmount.front() == '0' && isdigit(outgoingAmount.back())){
            throw ValueError("ResetTrustLineCommand: outgoingAmount contains leading zero.");
        }
        outgoingAmount += _attr(ctx);
        flagOutgoingAmount++;
        if (flagOutgoingAmount > 1 && outgoingAmount.front() == '0') {
            throw ValueError("ResetTrustLineCommand: outgoingAmount contains leading zero.");
        }
    };
    auto balanceSign = [&](auto &ctx) {
        if (_attr(ctx) != '-') {
            throw ValueError("ResetTrustLineCommand: balance contains invalid symbol.");
        }
        if (isBalanceNegative) {
            throw ValueError("ResetTrustLineCommand: balance contains minus several times.");
        }
        isBalanceNegative = true;
    };
    auto balanceAddNumber = [&](auto &ctx) {
        if(balance.front() == '0' && isdigit(balance.back())){
            throw ValueError("ResetTrustLineCommand: balance contains leading zero.");
        }
        balance += _attr(ctx);
        flagBalance++;
        if (flagBalance > 1 && balance.front() == '0') {
            throw ValueError("ResetTrustLineCommand: balance contains leading zero.");
        }
    };
    auto equivalentParse = [&](auto &ctx) {
        mEquivalent = _attr(ctx);
    };

    try {
        parse(
            command.begin(),
            command.end(),
            char_[check]);
        parse(
            command.begin(),
            command.end(),
            +(int_[contractorIDParse])
            > char_(kTokensSeparator)
            > +(int_[auditNumberParse])
            > char_(kTokensSeparator)
            > +(digit [incomingAmountAddNumber] > !alpha > !punct)
            > char_(kTokensSeparator)
            > +(digit [outgoingAmountAddNumber] > !alpha > !punct)
            > char_(kTokensSeparator)
            > *(char_('-') [balanceSign])
            > +(digit [balanceAddNumber] > !alpha > !punct)
            > char_(kTokensSeparator)
            > int_[equivalentParse] > eol > eoi );
        mIncomingTrustAmount = TrustLineAmount(incomingAmount);
        mOutgoingTrustAmount = TrustLineAmount(outgoingAmount);
        if (isBalanceNegative) {
            balance = "-" + balance;
        }
        mBalance = TrustLineBalance(balance);
    } catch(...) {
        throw ValueError("ResetTrustLineCommand : cannot parse command.");
    }
}

const string &ResetTrustLineCommand::identifier()
{
    static const string identifier = "SET:contractors/trust-lines/reset";
    return identifier;
}

const ContractorID ResetTrustLineCommand::contractorID() const
{
    return mContractorID;
}

const AuditNumber ResetTrustLineCommand::auditNumber() const
{
    return mAuditNumber;
}

const TrustLineAmount &ResetTrustLineCommand::incomingTrustAmount() const
{
    return mIncomingTrustAmount;
}

const TrustLineAmount& ResetTrustLineCommand::outgoingTrustAmount() const
{
    return mOutgoingTrustAmount;
}

const TrustLineBalance& ResetTrustLineCommand::balance() const
{
    return mBalance;
}

const SerializedEquivalent ResetTrustLineCommand::equivalent() const
{
    return mEquivalent;
}