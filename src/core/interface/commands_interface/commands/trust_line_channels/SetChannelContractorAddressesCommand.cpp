#include "SetChannelContractorAddressesCommand.h"

SetChannelContractorAddressesCommand::SetChannelContractorAddressesCommand(
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
            throw ValueError("SetChannelContractorAddressesCommand: there is no input ");
        }
    };
    auto contractorChannelIDParse = [&](auto &ctx) {
        mContractorChannelID = _attr(ctx);
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
            case BaseAddress::GNS: {
                mContractorAddresses.push_back(
                    make_shared<GNSAddress>(
                        address));
                addressType.erase();
                break;
            }
            default:
                throw ValueError("SetChannelContractorAddressesCommand: can't parse command. "
                                 "Error occurred while parsing 'Contractor Address' token.");
        }
        address.erase();
    };

    try {
        parse(
            command.begin(),
            command.end(),
            char_[check]);
        parse(
            command.begin(),
            command.end(),
            +(int_[contractorChannelIDParse]) > char_(kTokensSeparator)
            >+(int_[addressesCountParse]) > char_(kTokensSeparator));
        mContractorAddresses.reserve(addressesCount);
        parse(
            command.begin(),
            command.end(),
            +(int_) > char_(kTokensSeparator)
            >+(int_) > char_(kTokensSeparator)
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
            > eoi);

    } catch(...) {
        throw ValueError("SetChannelContractorAddressesCommand: can't parse command.");
    }
}

const string &SetChannelContractorAddressesCommand::identifier()
{
    static const string identifier = "SET:channel/address";
    return identifier;
}

vector<BaseAddress::Shared> SetChannelContractorAddressesCommand::contractorAddresses() const
{
    return mContractorAddresses;
}

const ContractorID SetChannelContractorAddressesCommand::contractorChannelID() const
{
    return mContractorChannelID;
}