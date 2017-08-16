#include "SubsystemsInfluenceCommand.h"

SubsystemsInfluenceCommand::SubsystemsInfluenceCommand(
    const CommandUUID &uuid,
    const string &commandBuffer) :

    BaseUserCommand(
        uuid,
        identifier())
{
    if (commandBuffer.size() == 0) {
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
        string hexUUID = commandBuffer.substr(
            tokenSeparatorPos + 1,
            NodeUUID::kHexSize);
        mForbiddenNodeUUID = boost::lexical_cast<uuids::uuid>(hexUUID);
    } else {
        mFlags = std::stoul(commandBuffer);
        mForbiddenNodeUUID = NodeUUID::empty();
    }
}

const string& SubsystemsInfluenceCommand::identifier()
{
    static const string identifier = "SET:tests/network/toggle";
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
