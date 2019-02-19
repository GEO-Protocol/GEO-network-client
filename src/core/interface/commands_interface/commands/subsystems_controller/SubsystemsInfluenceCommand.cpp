#include "SubsystemsInfluenceCommand.h"

SubsystemsInfluenceCommand::SubsystemsInfluenceCommand(
    const CommandUUID &uuid,
    const string &commandBuffer) :

    BaseUserCommand(
        uuid,
        identifier())
{
    uint32_t flag = 0, flag_4 = 0, flag_8 = 0, flag_12 = 0;
    std::string hexUUID, forbiddenamount("0");
    auto check = [&](auto &ctx) {
        if(_attr(ctx) == kCommandsSeparator) {
            throw ValueError("SubsystemsInfluenceCommand: there is no input ");
        }
    };
    auto flags_add = [&](auto &ctx) {
        mFlags = _attr(ctx);
    };
    auto mforbiddenamount_add = [&](auto &ctx) {
        forbiddenamount += _attr(ctx);
        flag++;
        if(flag>39) {
            throw ValueError("Amount is too big");
        } else if (flag == 1 && _attr(ctx) <= 0) {
            throw ValueError("Amount can't be zero or low");
        }
    };
    auto add_8 = [&](auto &ctx) {
        flag_8++;
        if (flag_8 > 9) {
            throw 1;
        } else if (_attr(ctx) == '-' && flag_8 < 9) {
            throw ValueError("Expect 8 digits");
        }
        hexUUID += _attr(ctx);
    };
    auto add_4 = [&](auto &ctx) {
        flag_4++;
        if (flag_4 > 5 || (_attr(ctx) == '-' && flag_4 < 5)) {
            throw ValueError("Expect 4 digits");
        } else if (_attr(ctx) == '-') {
            flag_4 = 0;
        }
        hexUUID += _attr(ctx);
    };
    auto add_12 = [&](auto &ctx) {
        flag_12++;
        if (flag_12 > 13 || (_attr(ctx) == kTokensSeparator && flag_12 < 13)) {
            throw ValueError("Expect 12 digits");
        } else if (_attr(ctx) == kTokensSeparator) {
        } else { hexUUID += _attr(ctx);
        }
    };

    try {
        parse(
            commandBuffer.begin(),
            commandBuffer.end(),
            char_[check]);
        parse(
            commandBuffer.begin(),
            commandBuffer.end(), (
                *(int_[flags_add])
                > -(char_(kTokensSeparator)
                > *(char_[add_8] - char_('-')) > char_('-')[add_8]
                > *(char_[add_4] - char_('-')) > char_('-')[add_4]
                > *(char_[add_4] - char_('-')) > char_('-')[add_4]
                > *(char_[add_4] - char_('-')) > char_('-')[add_4]
                > *(char_[add_12] - char_(kTokensSeparator))> char_(kTokensSeparator)[add_12]
                >*(digit [mforbiddenamount_add] > !alpha > !punct))
                >eol));
        if(flag_8 > 0) {
            mForbiddenNodeUUID = boost::lexical_cast<uuids::uuid>(hexUUID);
        } else {
            mForbiddenNodeUUID = NodeUUID::empty();
        }
      mForbiddenAmount = TrustLineAmount(forbiddenamount);
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

const NodeUUID& SubsystemsInfluenceCommand::forbiddenNodeUUID() const
{
    return mForbiddenNodeUUID;
}

const TrustLineAmount& SubsystemsInfluenceCommand::forbiddenAmount() const
{
    return mForbiddenAmount;
}
