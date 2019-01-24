#include "ShareKeysCommand.h"

ShareKeysCommand::ShareKeysCommand(
    const CommandUUID &commandUUID,
    const string &command) :

    BaseUserCommand(
        commandUUID,
        identifier())
{


    auto contractorID_add = [&](auto &ctx) { mContractorID = _attr(ctx); };
    auto equivalent_add = [&](auto &ctx) { mEquivalent = _attr(ctx); };

    try
    {
        parse(command.begin(), command.end(), *(int_[contractorID_add]) > char_('\t')  > int_[equivalent_add] > eol);
    }
    catch (...)
    {
        throw ValueError("SetOutgoingTrustLineCommand : can't parse command");
    }
}

const string &ShareKeysCommand::identifier()
    noexcept
{
    static const string identifier = "SET:contractors/trust-line-keys";
    return identifier;
}

const ContractorID ShareKeysCommand::contractorID() const
    noexcept
{
    return mContractorID;
}

const SerializedEquivalent ShareKeysCommand::equivalent() const
    noexcept
{
    return mEquivalent;
}