#include "GetFirstLevelContractorsCommand.h"

GetFirstLevelContractorsCommand::GetFirstLevelContractorsCommand(
    const CommandUUID &uuid,
    const string& commandBuffer):

    BaseUserCommand(
        uuid,
        identifier())
{
    auto check = [&](auto &ctx) {
        if(_attr(ctx) == kCommandsSeparator) {
            throw ValueError("GetFirstLevelContractorsCommand: there is no input ");
        }
    };
    auto equivalentParse = [&](auto &ctx) {
        mEquivalent = _attr(ctx);
    };

    try {
        parse(
            commandBuffer.begin(),
            commandBuffer.end(),
            char_[check]);
        parse(
            commandBuffer.begin(),
            commandBuffer.end(),
            *(int_[equivalentParse]) > kCommandsSeparator);
    } catch (...) {
        throw ValueError("GetFirstLevelContractorsCommand: can't parse command");
    }
}

const string &GetFirstLevelContractorsCommand::identifier()
{
    static const string identifier = "GET:contractors";
    return identifier;
}

const SerializedEquivalent GetFirstLevelContractorsCommand::equivalent() const
{
    return mEquivalent;
}

CommandResult::SharedConst GetFirstLevelContractorsCommand::resultOk(
    string& neighbors) const
{
    return make_shared<const CommandResult>(
        identifier(),
        UUID(),
        200,
        neighbors);
}

