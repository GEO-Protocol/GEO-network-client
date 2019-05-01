#include "InitTrustLineCommand.h"

InitTrustLineCommand::InitTrustLineCommand(
    const CommandUUID &commandUUID,
    const string &command) :
    BaseUserCommand(
        commandUUID,
        identifier())
{
    auto check = [&](auto &ctx) {
        if(_attr(ctx) == kCommandsSeparator || _attr(ctx) == kTokensSeparator) {
            throw ValueError("InitTrustLineCommand: input is empty.");
        }
    };
    auto contractorIDParse = [&](auto& ctx) {
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
            *(int_[contractorIDParse]) > char_(kTokensSeparator)
            > *(int_[equivalentParse]) > eol > eoi);
    } catch(...) {
        throw ValueError("InitTrustLineCommand: cannot parse command.");
    }
}

const string &InitTrustLineCommand::identifier()
{
    static const string identifier = "INIT:contractors/trust-line";
    return identifier;
}

const ContractorID InitTrustLineCommand::contractorID() const
{
    return mContractorID;
}

const SerializedEquivalent InitTrustLineCommand::equivalent() const
{
    return mEquivalent;
}