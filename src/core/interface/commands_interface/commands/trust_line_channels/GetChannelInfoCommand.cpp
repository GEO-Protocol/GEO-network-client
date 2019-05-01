#include "GetChannelInfoCommand.h"

GetChannelInfoCommand::GetChannelInfoCommand(
    const CommandUUID &commandUUID,
    const string &command) :
    BaseUserCommand(
        commandUUID,
        identifier())
{
    auto check = [&](auto &ctx) {
        if(_attr(ctx) == kCommandsSeparator) {
            throw ValueError("GetChannelInfoCommand: there is no input ");
        }
    };
    auto contractorIDParse = [&](auto& ctx) {
        mContractorID = _attr(ctx);
    };

    try {
        parse(
            command.begin(),
            command.end(),
            char_[check]);
        parse(
            command.begin(),
            command.end(),
            +(int_[contractorIDParse]) > eol > eoi );
    } catch(...) {
        throw ValueError("GetChannelInfoCommand: cannot parse command.");
    }
}

const string &GetChannelInfoCommand::identifier()
{
    static const string identifier = "GET:channels/one";
    return identifier;
}

const ContractorID GetChannelInfoCommand::contractorID() const
{
    return mContractorID;
}

CommandResult::SharedConst GetChannelInfoCommand::resultOk(
    string &neighbor) const
{
    return make_shared<const CommandResult>(
        identifier(),
        UUID(),
        200,
        neighbor);
}