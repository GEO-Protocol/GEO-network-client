#include "SetChannelContractorCryptoKeyCommand.h"

SetChannelContractorCryptoKeyCommand::SetChannelContractorCryptoKeyCommand(
    const CommandUUID &commandUUID,
    const string &command) :
    BaseUserCommand(
        commandUUID,
        identifier())
{
    auto check = [&](auto &ctx) {
        if(_attr(ctx) == kCommandsSeparator) {
            throw ValueError("SetChannelContractorCryptoKeyCommand: there is no input ");
        }
    };
    auto cryptoKeyParse = [&](auto &ctx) {
        mCryptoKey += _attr(ctx);
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
            +(int_[contractorChannelIDParse]-char_(kTokensSeparator)) > char_(kTokensSeparator)
            >+(char_[cryptoKeyParse] - eol)
            > eol > eoi);

    } catch(...) {
        throw ValueError("SetChannelContractorCryptoKeyCommand: can't parse command.");
    }
}

const string &SetChannelContractorCryptoKeyCommand::identifier()
{
    static const string identifier = "SET:channel/crypto-key";
    return identifier;
}

const string &SetChannelContractorCryptoKeyCommand::cryptoKey() const
{
    return mCryptoKey;
}

const ContractorID SetChannelContractorCryptoKeyCommand::contractorChannelID() const
{
    return mContractorChannelID;
}