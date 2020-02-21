#include "RemoveChannelCommand.h"

RemoveChannelCommand::RemoveChannelCommand(
    const CommandUUID &commandUUID,
    const string &command) :
    BaseUserCommand(
        commandUUID,
        identifier())
{
    auto check = [&](auto &ctx) {
        if(_attr(ctx) == kCommandsSeparator) {
            throw ValueError("RemoveChannelCommand: there is no input ");
        }
    };
    auto contractorChannelIDParse = [&](auto &ctx) {
        mContractorChannelID = _attr(ctx);
    };

    try {
        parse(
            command.begin(),
            command.end(),
            char_[check]);
        parse(
            command.begin(),
            command.end(),
            +(int_[contractorChannelIDParse]-eol)
            > eol > eoi);

    } catch(...) {
        throw ValueError("RemoveChannelCommand: can't parse command.");
    }
}

const string &RemoveChannelCommand::identifier()
{
    static const string identifier = "DELETE:channel/contractor-id";
    return identifier;
}

const ContractorID RemoveChannelCommand::contractorChannelID() const
{
    return mContractorChannelID;
}