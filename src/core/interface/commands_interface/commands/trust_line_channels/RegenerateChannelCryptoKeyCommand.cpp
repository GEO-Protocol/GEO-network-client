#include "RegenerateChannelCryptoKeyCommand.h"

RegenerateChannelCryptoKeyCommand::RegenerateChannelCryptoKeyCommand(
    const CommandUUID &commandUUID,
    const string &command) :
    BaseUserCommand(
        commandUUID,
        identifier())
{
    auto check = [&](auto &ctx) {
        if(_attr(ctx) == kCommandsSeparator) {
            throw ValueError("RegenerateChannelCryptoKeyCommand: there is no input ");
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
        throw ValueError("RegenerateChannelCryptoKeyCommand: can't parse command.");
    }
}

const string &RegenerateChannelCryptoKeyCommand::identifier()
{
    static const string identifier = "SET:channel/regenerate-crypto-key";
    return identifier;
}

const ContractorID RegenerateChannelCryptoKeyCommand::contractorChannelID() const
{
    return mContractorChannelID;
}

CommandResult::SharedConst RegenerateChannelCryptoKeyCommand::responseOk(
    string &channelInfo) const
{
    return CommandResult::SharedConst(
        new CommandResult(
            identifier(),
            UUID(),
            200,
            channelInfo));
}