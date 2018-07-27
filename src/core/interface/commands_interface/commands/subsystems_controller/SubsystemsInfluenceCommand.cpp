#include "SubsystemsInfluenceCommand.h"

SubsystemsInfluenceCommand::SubsystemsInfluenceCommand(
    const CommandUUID &uuid,
    const string &commandBuffer) :

    BaseUserCommand(
        uuid,
        identifier())
{
    if (commandBuffer.empty()) {
        throw ValueError(
            "SubsystemsInfluenceCommand: can't parse command."
            "Received command buffer is too short.");
    }

    auto tokenSeparatorPos = commandBuffer.find(
        kTokensSeparator);
    if (tokenSeparatorPos != std::string::npos) {
        auto flagsStr = commandBuffer.substr(
            0,
            tokenSeparatorPos);
        mFlags = std::stoul(flagsStr);

        auto hexUUID = commandBuffer.substr(
            tokenSeparatorPos + 1,
            NodeUUID::kHexSize);
        mForbiddenNodeUUID = boost::lexical_cast<uuids::uuid>(hexUUID);

        tokenSeparatorPos = commandBuffer.find(
            kTokensSeparator,
            tokenSeparatorPos + 1);
        auto forbiddenAmountStr = commandBuffer.substr(
            tokenSeparatorPos + 1,
            commandBuffer.size() - tokenSeparatorPos - 2);
        mForbiddenAmount = TrustLineAmount(forbiddenAmountStr);
    } else {
        mFlags = std::stoul(commandBuffer);
        mForbiddenNodeUUID = NodeUUID::empty();
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

const NodeUUID& SubsystemsInfluenceCommand::forbiddenNodeUUID() const
{
    return mForbiddenNodeUUID;
}

const TrustLineAmount& SubsystemsInfluenceCommand::forbiddenAmount() const
{
    return mForbiddenAmount;
}
