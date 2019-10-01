#include "GetChannelInfoByAddressesCommand.h"

GetChannelInfoByAddressesCommand::GetChannelInfoByAddressesCommand(
    const CommandUUID &uuid,
    const string &commandBuffer):

    BaseUserCommand(
        uuid,
        identifier())
{
    std::string address, addressType;
    uint32_t addressesCount;
    auto check = [&](auto &ctx) {
        if(_attr(ctx) == kCommandsSeparator || _attr(ctx) == kTokensSeparator) {
            throw ValueError("GetChannelInfoByAddressesCommand: there is no input ");
        }
    };
    auto addressTypeParse = [&](auto &ctx) {
        addressType += _attr(ctx);
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
                throw ValueError("GetChannelInfoByAddressesCommand: cannot parse command. "
                                 "Error occurred while parsing 'Contractor Address' token.");
        }
        address.erase();
    };

    try {
        parse(
            commandBuffer.begin(),
            commandBuffer.end(),
            char_[check]);
        parse(
            commandBuffer.begin(),
            commandBuffer.end(),
            *(int_[addressesCountParse]-char_(kTokensSeparator)) > char_(kTokensSeparator));
        parse(
            commandBuffer.begin(),
            commandBuffer.end(), (
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
                > eoi));
    } catch(...) {
        throw ValueError("GetChannelInfoByAddressesCommand: cannot parse command.");
    }
}

const string &GetChannelInfoByAddressesCommand::identifier()
{
    static const string kIdentifier = "GET:channels/one/address";
    return kIdentifier;
}

vector<BaseAddress::Shared> GetChannelInfoByAddressesCommand::contractorAddresses() const
{
    return mContractorAddresses;
}

CommandResult::SharedConst GetChannelInfoByAddressesCommand::resultOk(
    string &neighbor) const
{
    return make_shared<const CommandResult>(
        identifier(),
        UUID(),
        200,
        neighbor);
}
