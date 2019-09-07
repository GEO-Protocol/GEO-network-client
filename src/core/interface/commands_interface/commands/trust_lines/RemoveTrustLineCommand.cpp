#include "RemoveTrustLineCommand.h"

RemoveTrustLineCommand::RemoveTrustLineCommand(
    const CommandUUID &commandUUID,
    const string &command) :

    BaseUserCommand(
        commandUUID,
        identifier())
{
    auto check = [&](auto &ctx) {
        if(_attr(ctx) == kCommandsSeparator || _attr(ctx) == kTokensSeparator) {
            throw ValueError("RemoveTrustLineCommand: input is empty.");
        }
    };
    auto contractorIDParse = [&](auto &ctx) {
        mContractorID = _attr(ctx);
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
            > *(int_[equivalentParse])
            > eol > eoi);
    } catch (...) {
        throw ValueError("RemoveTrustLineCommand: cannot parse command.");
    }
}

const string &RemoveTrustLineCommand::identifier()
{
    static const string identifier = "DELETE:contractors/trust-line";
    return identifier;
}

const ContractorID RemoveTrustLineCommand::contractorID() const
{
    return mContractorID;
}

const SerializedEquivalent RemoveTrustLineCommand::equivalent() const
{
    return mEquivalent;
}