#include "ShareKeysCommand.h"

ShareKeysCommand::ShareKeysCommand(
    const CommandUUID &commandUUID,
    const string &command) :

    BaseUserCommand(
        commandUUID,
        identifier())
{
    auto check = [&](auto &ctx) {
        if(_attr(ctx) == kCommandsSeparator || _attr(ctx) == kTokensSeparator) {
            throw ValueError("ShareKeysCommand: input is empty.");
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
            *(int_[contractorIDParse]) > char_(kTokensSeparator)  > int_[equivalentParse] > eol > eoi);
    } catch (...) {
        throw ValueError("ShareKeysCommand: cannot parse command.");
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