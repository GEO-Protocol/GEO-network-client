#include "GetTrustLineByIDCommand.h"

GetTrustLineByIDCommand::GetTrustLineByIDCommand(
    const CommandUUID &commandUUID,
    const string &command) :
    BaseUserCommand(
        commandUUID,
        identifier())
{
    auto check = [&](auto &ctx) {
        if(_attr(ctx) == kCommandsSeparator) {
            throw ValueError("GetTrustLineByIDCommand: there is no input ");
        }
    };
    auto contractorIDParse = [&](auto& ctx) {
        mContractorID = _attr(ctx);
    };
    auto equivalentParse = [&](auto& ctx) {
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
            *(int_[contractorIDParse]) > char_(kTokensSeparator) > *(int_[equivalentParse]) > eol > eoi );
    } catch(...) {
        throw ValueError("GetTrustLineByIDCommand: cannot parse command.");
    }
}

const string &GetTrustLineByIDCommand::identifier()
{
    static const string identifier = "GET:contractors/trust-lines/one/id";
    return identifier;
}

const ContractorID GetTrustLineByIDCommand::contractorID() const
{
    return mContractorID;
}

const SerializedEquivalent GetTrustLineByIDCommand::equivalent() const
{
    return mEquivalent;
}

CommandResult::SharedConst GetTrustLineByIDCommand::resultOk(
    string &neighbor) const
{
    return make_shared<const CommandResult>(
        identifier(),
        UUID(),
        200,
        neighbor);
}