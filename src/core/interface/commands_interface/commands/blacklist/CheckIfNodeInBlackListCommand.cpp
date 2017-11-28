#include "CheckIfNodeInBlackListCommand.h"


CheckIfNodeInBlackListCommand::CheckIfNodeInBlackListCommand(
    const CommandUUID &commandUUID,
    const string &command) :

    BaseUserCommand(
        commandUUID,
        identifier())
{
    static const auto minCommandLength = NodeUUID::kHexSize + 1;

    if (command.size() < minCommandLength) {
        throw ValueError(
            "CheckIfNodeInBlackListCommand: can't parse command. "
                "Received command is to short.");
    }

    try {
        string hexUUID = command.substr(0, NodeUUID::kHexSize);
        mContractorUUID = boost::lexical_cast<uuids::uuid>(hexUUID);

    } catch (...) {
        throw ValueError(
            "CheckIfNodeInBlackListCommand: can't parse command. "
                "Error occurred while parsing 'Contractor UUID' token.");
    }
}

const string &CheckIfNodeInBlackListCommand::identifier()
noexcept
{
    static const string identifier = "GET:blacklist/check";
    return identifier;
}

const NodeUUID &CheckIfNodeInBlackListCommand::contractorUUID()
noexcept {
    return mContractorUUID;
}