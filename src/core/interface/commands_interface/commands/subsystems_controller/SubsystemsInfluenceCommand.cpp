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

    mFlags = std::stoul(commandBuffer);
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
