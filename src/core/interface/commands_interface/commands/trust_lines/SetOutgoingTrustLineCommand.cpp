#include "SetOutgoingTrustLineCommand.h"


SetOutgoingTrustLineCommand::SetOutgoingTrustLineCommand(
    const CommandUUID &commandUUID,
    const string &command) :

    BaseUserCommand(
        commandUUID,
        identifier())
{

    auto contractorID_add = [&](auto &ctx) { mContractorID = _attr(ctx); };
    auto amount_add = [&](auto &ctx) { mAmount = _attr(ctx); };
    auto equivalent_add = [&](auto &ctx) { mEquivalent = _attr(ctx); };

    try
    {
        parse(command.begin(), command.end(), *(int_[contractorID_add]) > char_('\t')  > *(int_[amount_add]) > char_('\t')  > int_[equivalent_add] > eol );
    }
    catch(...)
    {
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
