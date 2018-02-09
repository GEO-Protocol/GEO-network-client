#include "GetBlackListCommand.h"

GetBlackListCommand::GetBlackListCommand(
    const CommandUUID &uuid,
    const string& commandBuffer)
    noexcept:
    BaseUserCommand(
        uuid,
        identifier())
{}

const string &GetBlackListCommand::identifier()
{
    static const string identifier = "GET:blacklist";
    return identifier;
}

CommandResult::SharedConst GetBlackListCommand::resultOk(
    string& bannedUsers) const
{
    return make_shared<const CommandResult>(
        identifier(),
        UUID(),
        200,
        bannedUsers);
}

