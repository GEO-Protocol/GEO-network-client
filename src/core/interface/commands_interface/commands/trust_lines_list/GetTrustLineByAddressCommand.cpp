#include "GetTrustLineByAddressCommand.h"

GetTrustLineByAddressCommand::GetTrustLineByAddressCommand(
    const CommandUUID &uuid,
    const string &commandBuffer):

    BaseUserCommand(
        uuid,
        identifier())
{
    std::string address;
    uint32_t addressType, addressesCount, equivalentID;
    auto check = [&](auto &ctx) {
        if(_attr(ctx) == kCommandsSeparator) {
            throw ValueError("GetTrustLineByAddressCommand: there is no input ");
        }
    };
    auto parserType = [&](auto &ctx) {
        addressType = _attr(ctx);
    };
    auto address_add = [&](auto &ctx) {
        address += _attr(ctx);
    };
    auto address_number_add = [&](auto &ctx) {
        address += std::to_string(_attr(ctx));
    };
    auto address_Count = [&](auto &ctx) {
        addressesCount = _attr(ctx);
    };
    auto address_vector = [&](auto &ctx) {
        switch (addressType) {
            case BaseAddress::IPv4_IncludingPort: {
                mContractorAddresses.push_back(
                    make_shared<IPv4WithPortAddress>(
                        address));
                break;
            }
            default:
                throw ValueError("GetTrustLineByAddressCommand: can't parse command. "
                    "Error occurred while parsing 'Contractor Address' token.");
        }
        address.erase();
    };
    auto equivalentID_add = [&](auto &ctx) {
        equivalentID = _attr(ctx);
    };

    try {
        parse(
            commandBuffer.begin(),
            commandBuffer.end(),
            char_[check]);
        parse(
            commandBuffer.begin(),
            commandBuffer.end(),
            *(int_[address_Count]-char_(kTokensSeparator)) > char_(kTokensSeparator));
        parse(
            commandBuffer.begin(),
            commandBuffer.end(), (
                *(int_[address_Count]) > char_(kTokensSeparator)
                > repeat(addressesCount)[*(int_[parserType] - char_(kTokensSeparator)) > char_(kTokensSeparator)
                > repeat(3)[int_[address_number_add]> char_('.') [address_add]]
                > int_[address_number_add] > char_(':') [address_add]
                > int_[address_number_add] > char_(kTokensSeparator) [address_vector]]
                > +(int_[equivalentID_add]) > eol));
    } catch(...) {
        throw ValueError("GetTrustLineByAddressCommand: can't parse command.");
    }
    mEquivalent = equivalentID;
}

const string &GetTrustLineByAddressCommand::identifier()
{
    static const string kIdentifier = "GET:contractors/trust-lines/one/address";
    return kIdentifier;
}

vector<BaseAddress::Shared> GetTrustLineByAddressCommand::contractorAddresses() const
{
    return mContractorAddresses;
}

const SerializedEquivalent GetTrustLineByAddressCommand::equivalent() const
{
    return mEquivalent;
}

CommandResult::SharedConst GetTrustLineByAddressCommand::resultOk(
    string &neighbor) const
{
    return make_shared<const CommandResult>(
        identifier(),
        UUID(),
        200,
        neighbor);
}
