#include "SubsystemsInfluenceCommand.h"

SubsystemsInfluenceCommand::SubsystemsInfluenceCommand(
    const CommandUUID &uuid,
    const string &commandBuffer) :

    BaseUserCommand(
        uuid,
        identifier())
{
//    uint32_t flag = 0, addressType;
//    std::string forbiddenamount("0"), address;
//    mForbiddenNodeAddress = nullptr;
//    auto check = [&](auto &ctx) {
//        if(_attr(ctx) == kCommandsSeparator) {
//            throw ValueError("SubsystemsInfluenceCommand: there is no input ");
//        }
//    };
//    auto flags_add = [&](auto &ctx) {
//        mFlags = _attr(ctx);
//    };
//    auto mforbiddenamount_add = [&](auto &ctx) {
//        forbiddenamount += _attr(ctx);
//        flag++;
//        if(flag > 39) {
//            throw ValueError("Amount is too big");
//        } else if (flag == 1 && _attr(ctx) <= 0) {
//            throw ValueError("Amount can't be zero or low");
//        }
//    };
//    auto parserType = [&](auto &ctx) {
//        addressType = _attr(ctx);
//    };
//    auto address_add = [&](auto &ctx) {
//        address += _attr(ctx);
//    };
//    auto address_number_add = [&](auto &ctx) {
//        address += std::to_string(_attr(ctx));
//    };
//    auto address_vector = [&](auto &ctx) {
//        switch (addressType) {
//            case BaseAddress::IPv4_IncludingPort: {
//                mForbiddenNodeAddress = make_shared<IPv4WithPortAddress>(address);
//                break;
//            }
//            default:
//                throw ValueError("SubsystemsInfluenceCommand: can't parse command. "
//                    "Error occurred while parsing 'Contractor Address' token.");
//        }
//        address.erase();
//    };
//
//    try {
//        parse(
//            commandBuffer.begin(),
//            commandBuffer.end(),
//            char_[check]);
//        parse(
//            commandBuffer.begin(),
//            commandBuffer.end(), (
//                *(int_[flags_add])
//                > -(char_(kTokensSeparator)
//                > *(int_[parserType] - char_(kTokensSeparator)) > char_(kTokensSeparator)
//                  > repeat(3)[int_[address_number_add]> char_('.') [address_add]]
//                  > int_[address_number_add] > char_(':') [address_add]
//                  > int_[address_number_add] > char_(kTokensSeparator) [address_vector]
//                  > char_(kTokensSeparator)
//                >*(digit [mforbiddenamount_add] > !alpha > !punct))
//                > eol));
//
//        mForbiddenAmount = TrustLineAmount(forbiddenamount);
//    } catch (...) {
//        throw ValueError("SubsystemsInfluenceCommand: can't parse command");
//    }


    if (commandBuffer.empty()) {
        throw ValueError("SubsystemsInfluenceCommand: can't parse command."
            "Received command buffer is too short.");
    }

    auto tokenSeparatorPos = commandBuffer.find(
        kTokensSeparator);
    if (tokenSeparatorPos != std::string::npos) {
        auto flagsStr = commandBuffer.substr(
            0,
            tokenSeparatorPos);
        mFlags = std::stoul(flagsStr);

        auto addressStartPos = tokenSeparatorPos + 1;
        tokenSeparatorPos = commandBuffer.find(
            kTokensSeparator,
            addressStartPos);
        string addressTypeStr = commandBuffer.substr(
            addressStartPos,
            tokenSeparatorPos);

        try {
            auto addressType = (BaseAddress::AddressType) std::stoul(addressTypeStr);
            auto contractorAddressStartPos = tokenSeparatorPos + 1;
            tokenSeparatorPos = commandBuffer.find(
                kTokensSeparator,
                contractorAddressStartPos);
            string addressStr = commandBuffer.substr(
                contractorAddressStartPos,
                tokenSeparatorPos - contractorAddressStartPos);
            switch (addressType) {
                case BaseAddress::IPv4_IncludingPort: {
                    mForbiddenNodeAddress = make_shared<IPv4WithPortAddress>(
                        addressStr);
                    break;
                }
                default:
                    throw ValueError("SubsystemsInfluenceCommand: can't parse command. "
                        "Error occurred while parsing 'Contractor Address' token.");
            }
        } catch (...) {
            throw ValueError("SubsystemsInfluenceCommand: can't parse command. "
                "Error occurred while parsing 'Contractor Address' token.");
        }

        auto forbiddenAmountStr = commandBuffer.substr(
            tokenSeparatorPos + 1,
            commandBuffer.size() - tokenSeparatorPos - 2)   ;
        mForbiddenAmount = TrustLineAmount(forbiddenAmountStr);
    } else {
        mFlags = std::stoul(commandBuffer);
        mForbiddenNodeAddress = nullptr;
        mForbiddenAmount = 0;
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
