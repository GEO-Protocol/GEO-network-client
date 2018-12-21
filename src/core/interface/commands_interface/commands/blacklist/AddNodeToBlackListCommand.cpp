#include "AddNodeToBlackListCommand.h"


AddNodeToBlackListCommand::AddNodeToBlackListCommand(
    const CommandUUID &commandUUID,
    const string &command) :

    BaseUserCommand(
        commandUUID,
        identifier())
{
    static const auto minCommandLength = 2;

    if (command.size() < minCommandLength) {
        throw ValueError(
            "AddNodeToBlackListCommand: can't parse command. "
                "Received command is to short.");
    }

    auto contractorAddressStr = command.substr(
        0,
        command.size() - 1);
    try {
        mContractorAddress = make_shared<IPv4WithPortAddress>(
            contractorAddressStr);
    } catch (...) {
        throw ValueError(
            "AddNodeToBlackListCommand: can't parse command. "
                "Error occurred while parsing 'Contractor Address' token.");
    }
}

const string &AddNodeToBlackListCommand::identifier()
    noexcept
{
    static const string identifier = "POST:blacklist";
    return identifier;
}

const NodeUUID &AddNodeToBlackListCommand::contractorUUID()
    noexcept
{
    return mContractorUUID;
}

BaseAddress::Shared AddNodeToBlackListCommand::contractorAddress()
    noexcept
{
    return mContractorAddress;
}