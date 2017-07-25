#include "ToggleNetworkCommand.h"

#ifdef TESTS

ToggleNetworkCommand::ToggleNetworkCommand(
    const CommandUUID &uuid,
    const string &commandBuffer) :

    BaseTestCommand(
        uuid,
        identifier())
{
    if (commandBuffer.size() == 0) {
        throw ValueError(
            "ToggleNetworkCommand: can't parse command."
            "Received command buffer is too short.");
    }

    mFlags = std::stoul(commandBuffer);
}

const string& ToggleNetworkCommand::identifier()
{
    static const string identifier = "SET:tests/network/toggle";
    return identifier;
}

size_t ToggleNetworkCommand::flags() const
{
    return mFlags;
}

#endif
