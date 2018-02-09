#include "RemoveNodeFromBlackListCommand.h"


RemoveNodeFromBlackListCommand::RemoveNodeFromBlackListCommand(
    const CommandUUID &commandUUID,
    const string &command) :

    BaseUserCommand(
        commandUUID,
        identifier())
{
    static const auto minCommandLength = NodeUUID::kHexSize + 1;

    if (command.size() < minCommandLength) {
        throw ValueError(
            "RemoveNodeFromBlackListCommand: can't parse command. "
                "Received command is to short.");
    }

    try {
        string hexUUID = command.substr(0, NodeUUID::kHexSize);
        mContractorUUID = boost::lexical_cast<uuids::uuid>(hexUUID);

    } catch (...) {
        throw ValueError(
            "RemoveNodeFromBlackListCommand: can't parse command. "
                "Error occurred while parsing 'Contractor UUID' token.");
    }
}

const string &RemoveNodeFromBlackListCommand::identifier()
    noexcept
{
    static const string identifier = "DELETE:blacklist";
    return identifier;
}

const NodeUUID &RemoveNodeFromBlackListCommand::contractorUUID()
    noexcept
{
    return mContractorUUID;
}
