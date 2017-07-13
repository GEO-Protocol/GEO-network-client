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

    mIsNetworkOn = (commandBuffer[0] == '1');
}

const string& ToggleNetworkCommand::identifier()
{
    static const string identifier = "SET:tests/network/toggle";
    return identifier;
}

bool ToggleNetworkCommand::isNetworkOn() const
{
    return mIsNetworkOn;
}

#endif
