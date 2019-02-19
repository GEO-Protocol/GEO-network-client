#include "SetOutgoingTrustLineCommand.h"


SetOutgoingTrustLineCommand::SetOutgoingTrustLineCommand(
    const CommandUUID &commandUUID,
    const string &command) :

    BaseUserCommand(
        commandUUID,
        identifier())
{
    std::string amount;
    uint32_t flag_amount=0;
    auto check = [&](auto &ctx) {
        if(_attr(ctx) == kCommandsSeparator) {
            throw ValueError("SetOutgoingTrustLineCommand: there is no input ");
        }
    };
    auto contractorID_add = [&](auto &ctx) {
        mContractorID = _attr(ctx);
    };
    auto amount_add = [&](auto &ctx) {
        amount += _attr(ctx);
        flag_amount++;
        if (flag_amount > 39) {
            throw ValueError("Amount is too big");
        } else if (flag_amount == 1 && _attr(ctx) <= 0) {
            throw ValueError("Amount can't be zero or low");
        }
    };
    auto equivalent_add = [&](auto &ctx) {
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
            *(int_[contractorID_add])
                > char_(kTokensSeparator)
                > *(digit [amount_add] > !alpha > !punct)
                > char_(kTokensSeparator)  > int_[equivalent_add] > eol );
    } catch(...) {
        throw ValueError("SetOutgoingTrustLineCommand : can't parse command");
    }
    mAmount = TrustLineAmount(amount);
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
