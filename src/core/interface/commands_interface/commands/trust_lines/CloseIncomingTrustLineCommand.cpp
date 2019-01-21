#include "CloseIncomingTrustLineCommand.h"

CloseIncomingTrustLineCommand::CloseIncomingTrustLineCommand(
    const CommandUUID &commandUUID,
    const string &command) :

    BaseUserCommand(
        commandUUID,
        identifier())
{

    auto contractorid_add = [&](auto& ctx) {mContractorID = _attr(ctx);};
    auto equivalentid_add = [&](auto& ctx) {mEquivalent = _attr(ctx);};

    parse(command.begin(), command.end(), (int_[contractorid_add] >> space >> int_[equivalentid_add]));

}

const string &CloseIncomingTrustLineCommand::identifier()
    noexcept
{
    static const string identifier = "DELETE:contractors/incoming-trust-line";
    return identifier;
}

const ContractorID CloseIncomingTrustLineCommand::contractorID() const
    noexcept
{
    return mContractorID;
}

const SerializedEquivalent CloseIncomingTrustLineCommand::equivalent() const
    noexcept
{
    return mEquivalent;
}
