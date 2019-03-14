#include "SubsystemsInfluenceCommand.h"

SubsystemsInfluenceCommand::SubsystemsInfluenceCommand(
    const CommandUUID &uuid,
    const string &commandBuffer) :

    BaseUserCommand(
        uuid,
        identifier())
{
    uint32_t flag = 0, addressType;
    std::string forbiddenAmount, address;
    mForbiddenNodeAddress = nullptr;
    auto check = [&](auto &ctx) {
        if(_attr(ctx) == kCommandsSeparator) {
            throw ValueError("SubsystemsInfluenceCommand: there is no input ");
        }
    };
    auto flagsAdd = [&](auto &ctx) {
        mFlags = _attr(ctx);
    };
    auto forbiddenAmountAdd = [&](auto &ctx) {
        forbiddenAmount += _attr(ctx);
        flag++;
        if(flag > 39) {
            throw ValueError("Amount is too big");
        } else if (flag == 1 && _attr(ctx) <= 0) {
            throw ValueError("Amount can't be zero or low");
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
    auto addressAddToVector = [&](auto &ctx) {
        switch (addressType) {
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
                    > *(int_[addressTypeParse] - char_(kTokensSeparator)) > char_(kTokensSeparator)
                    > repeat(3)[int_[addressAddNumber]> char_('.') [addressAddChar]]
                    > int_[addressAddNumber] > char_(':') [addressAddChar]
                    > int_[addressAddNumber] > char_(kTokensSeparator) [addressAddToVector]
                    >*(digit [forbiddenAmountAdd] > !alpha > !punct))
                > eol));

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
