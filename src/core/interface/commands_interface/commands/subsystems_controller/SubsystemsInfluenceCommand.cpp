#include "SubsystemsInfluenceCommand.h"

SubsystemsInfluenceCommand::SubsystemsInfluenceCommand(
    const CommandUUID &uuid,
    const string &commandBuffer) :

    BaseUserCommand(
        uuid,
        identifier())
{
    uint32_t flag = 0;
    std::string forbiddenAmount, address, addressType;
    mForbiddenNodeAddress = nullptr;
    auto check = [&](auto &ctx) {
        if(_attr(ctx) == kCommandsSeparator || _attr(ctx) == kTokensSeparator) {
            throw ValueError("SubsystemsInfluenceCommand: there is no input ");
        }
    };
    auto flagsAdd = [&](auto &ctx) {
        mFlags = _attr(ctx);
    };
    auto forbiddenAmountAdd = [&](auto &ctx) {
        if(forbiddenAmount.front() == '0') {throw ValueError("Amount start's from zero");}
        forbiddenAmount += _attr(ctx);
        flag++;
        if(flag >= 78) {
            for(int i = 0 ; i < forbiddenAmount.length(); i++)
            {
                if(forbiddenAmount[i] != kAmountLimit[i])
                {
                    throw ValueError("Amount is too big");
                }

            }
        }else if (flag == 1 && _attr(ctx) == '0') {
            throw ValueError("Amount can't be zero or low");
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
    auto addressAddToVector = [&](auto &ctx) {
        switch (std::atoi(addressType.c_str())) {
            case BaseAddress::IPv4_IncludingPort: {
                mForbiddenNodeAddress = make_shared<IPv4WithPortAddress>(address);
                break;
            }
            default:
                throw ValueError("SubsystemsInfluenceCommand: can't parse command. "
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
            commandBuffer.end(), (
                *(int_[flagsAdd])
                > -(char_(kTokensSeparator)
                    > expect
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
//                          parserString::string(std::to_string(<NEW_ADDRESS_TYPE>) [addressTypeParse]
//                          > *(char_[addressTypeParse] - char_(kTokensSeparator)
//                          > char_(kTokensSeparator)
//                          > <NEW_PARSE_RULE>
                    ]
                    > *(digit [forbiddenAmountAdd] > !alpha > !punct))
                > eol > eoi));

        mForbiddenAmount = TrustLineAmount(forbiddenAmount);
    } catch (...) {
        throw ValueError("SubsystemsInfluenceCommand: can't parse command");
    }
}

const string& SubsystemsInfluenceCommand::identifier()
{
    static const string identifier = "SET:subsystems_controller/flags";
    return identifier;
}

size_t SubsystemsInfluenceCommand::flags() const
{
    return mFlags;
}

BaseAddress::Shared SubsystemsInfluenceCommand::forbiddenNodeAddress() const
{
    return mForbiddenNodeAddress;
}

const TrustLineAmount& SubsystemsInfluenceCommand::forbiddenAmount() const
{
    return mForbiddenAmount;
}
