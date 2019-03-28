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
            throw ValueError("SubsystemsInfluenceCommand:  input is empty.");
        }
    };
    auto flagsAdd = [&](auto &ctx) {
        mFlags = _attr(ctx);
    };
    auto forbiddenAmountAdd = [&](auto &ctx) {
        forbiddenAmount += _attr(ctx);
        flag++;
        if (flag == 1 && _attr(ctx) == '0') {
            throw ValueError("SubsystemsInfluenceCommand: amount contains leading zero.");
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
                throw ValueError("SubsystemsInfluenceCommand: cannot parse command. "
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
                > addressLexeme<
                    decltype(addressAddChar),
                    decltype(addressAddNumber),
                    decltype(addressTypeParse),
                    decltype(addressAddToVector)>(
                        1,
                        addressAddChar,
                        addressAddNumber,
                        addressTypeParse,
                        addressAddToVector)
                    > *(digit [forbiddenAmountAdd] > !alpha > !punct))
                > eol > eoi));

        mForbiddenAmount = TrustLineAmount(forbiddenAmount);
    } catch (...) {
        throw ValueError("SubsystemsInfluenceCommand: cannot parse command.");
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
