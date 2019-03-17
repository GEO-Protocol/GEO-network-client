#include "InitTrustLineCommand.h"

InitTrustLineCommand::InitTrustLineCommand(
    const CommandUUID &commandUUID,
    const string &command) :
    BaseUserCommand(
        commandUUID,
        identifier())
{
    std::string address,addressType;
    uint32_t addressesCount;
    auto check = [&](auto &ctx) {
        if(_attr(ctx) == kCommandsSeparator || _attr(ctx) == kTokensSeparator) {
            throw ValueError("InitTrustLineCommand: there is no input ");
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
            default:
                throw ValueError("InitTrustLineCommand: can't parse command. "
                    "Error occurred while parsing 'Contractor Address' token.");
        }
        address.erase();
    };
    auto equivalentParse = [&](auto &ctx) {
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
            *(int_[addressesCountParse]-char_(kTokensSeparator)) > char_(kTokensSeparator));
        mContractorAddresses.reserve(addressesCount);
        parse(
            command.begin(),
            command.end(),
                *(int_) > char_(kTokensSeparator)
                > expect
                [
                        repeat(addressesCount)
                        [
                                parserString::string(std::to_string(BaseAddress::IPv4_IncludingPort)) [addressTypeParse]
                                > *(char_[addressTypeParse] - char_(kTokensSeparator))
                                >char_(kTokensSeparator)
                                > repeat(3)
                                [
                                        int_[addressAddNumber]
                                        > char_('.') [addressAddChar]
                                ]
                                > int_[addressAddNumber]
                                > char_(':') [addressAddChar]
                                > int_[addressAddNumber]
                                > char_(kTokensSeparator) [addressAddToVector]

//                                         | //OR
//
//                              parserString::string(std::to_string(<NEW_ADDRESS_TYPE>) [addressTypeParse]
//                              > *(char_[addressTypeParse] - char_(kTokensSeparator) )
//                              > char_(kTokensSeparator)
//                              > <NEW_PARSE_RULE>
                        ]
                ]
                  > *(int_[equivalentParse]) > eol > eoi);
    } catch(...) {
        throw ValueError("InitTrustLineCommand: can't parse command.");
    }
}

const string &InitTrustLineCommand::identifier()
noexcept
{
    static const string identifier = "INIT:contractors/trust-line";
    return identifier;
}

const SerializedEquivalent InitTrustLineCommand::equivalent() const
noexcept
{
    return mEquivalent;
}

vector<BaseAddress::Shared> InitTrustLineCommand::contractorAddresses() const
noexcept
{
    return mContractorAddresses;
}
