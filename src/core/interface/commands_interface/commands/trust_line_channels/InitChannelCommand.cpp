#include "InitChannelCommand.h"

InitChannelCommand::InitChannelCommand(
    const CommandUUID &commandUUID,
    const string &command) :
    BaseUserCommand(
        commandUUID,
        identifier())
{
    std::string address, addressType;
    uint32_t addressesCount;
    auto check = [&](auto &ctx) {
        if(_attr(ctx) == kCommandsSeparator) {
            throw ValueError("InitChannelCommand: there is no input ");
        }
    };
    auto addressTypeParse = [&](auto &ctx) {
        addressType = _attr(ctx);
    };
    auto addressAddChar = [&](auto &ctx) {
        address += _attr(ctx);
    };
    auto addressAddNumber = [&](auto &ctx) {
        address += std::to_string(_attr(ctx));
    };
    auto addressesCountParse = [&](auto &ctx) {
        addressesCount = _attr(ctx);
    };
    auto addressAddToVector = [&](auto &ctx) {
        switch (std::atoi(addressType.c_str())) {
            case BaseAddress::IPv4_IncludingPort: {
                mContractorAddresses.push_back(
                    make_shared<IPv4WithPortAddress>(
                        address));
                addressType.erase();
                break;
            }
            default:
                throw ValueError("InitChannelCommand: can't parse command. "
                                 "Error occurred while parsing 'Contractor Address' token.");
        }
        address.erase();
    };
    auto cryptoKeyParse = [&](auto &ctx) {
        mCryptoKey = _attr(ctx);
    };

    try {
        parse(
            command.begin(),
            command.end(),
            char_[check]);
        parse(
            command.begin(),
            command.end(),
            *(int_[addressesCountParse]-char_(kTokensSeparator)) > char_(kTokensSeparator));
        mContractorAddresses.reserve(addressesCount);
        parse(
            command.begin(),
            command.end(),
            *(int_) > char_(kTokensSeparator)
            > addressLexeme<
                decltype(addressAddChar),
                decltype(addressAddNumber),
                decltype(addressTypeParse),
                decltype(addressAddToVector)>(
                    addressesCount,
                    addressAddChar,
                    addressAddNumber,
                    addressTypeParse,
                    addressAddToVector)
                > -(int_[cryptoKeyParse] > eol)
                > eoi);
    } catch(...) {
        throw ValueError("InitChannelCommand: can't parse command.");
    }
}

const string &InitChannelCommand::identifier()
noexcept
{
    static const string identifier = "INIT:contractors/channel";
    return identifier;
}

vector<BaseAddress::Shared> InitChannelCommand::contractorAddresses() const
noexcept
{
    return mContractorAddresses;
}

uint32_t InitChannelCommand::cryptoKey() const
{
    return mCryptoKey;
}

CommandResult::SharedConst InitChannelCommand::responseOk(
    string &channelInfo) const
{
    return CommandResult::SharedConst(
        new CommandResult(
            identifier(),
            UUID(),
            200,
            channelInfo));
}