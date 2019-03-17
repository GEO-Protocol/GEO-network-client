#include "SetOutgoingTrustLineCommand.h"


SetOutgoingTrustLineCommand::SetOutgoingTrustLineCommand(
    const CommandUUID &commandUUID,
    const string &command) :

    BaseUserCommand(
        commandUUID,
        identifier())
{
    std::string amount;
    uint32_t flagAmount=0;
    auto check = [&](auto &ctx) {
        if(_attr(ctx) == kCommandsSeparator || _attr(ctx) == kTokensSeparator) {
            throw ValueError("SetOutgoingTrustLineCommand: there is no input ");
        }
    };
    auto contractorIDParse = [&](auto &ctx) {
        mContractorID = _attr(ctx);
    };
    auto amountAddNumber = [&](auto &ctx) {
        if(amount.front() == '0'&& isdigit(amount.back())){ throw ValueError("Amount can't start from zero");}
        amount += _attr(ctx);
        flagAmount++;
        if(flagAmount >= 78) {
            for(int i = 0 ; i < amount.length(); i++)
            {
                if(amount[i] != kAmountLimit[i])
                {
                    throw ValueError("Amount is too big");
                }

            }
        }else if (flagAmount == 1 && _attr(ctx) == '0') {
            throw ValueError("Amount can't be zero or low");
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
            *(int_[contractorIDParse])
                > char_(kTokensSeparator)
                > *(digit [amountAddNumber] > !alpha > !punct)
                > char_(kTokensSeparator)  > int_[equivalentParse] > eol > eoi );
        mAmount = TrustLineAmount(amount);
    } catch(...) {
        throw ValueError("SetOutgoingTrustLineCommand : can't parse command");
    }
}

const string &SetOutgoingTrustLineCommand::identifier()
    noexcept
{
    static const string identifier = "SET:contractors/trust-lines";
    return identifier;
}

const ContractorID SetOutgoingTrustLineCommand::contractorID() const
    noexcept
{
    return mContractorID;
}

const TrustLineAmount &SetOutgoingTrustLineCommand::amount() const
    noexcept
{
    return mAmount;
}

const SerializedEquivalent SetOutgoingTrustLineCommand::equivalent() const
    noexcept
{
    return mEquivalent;
}
