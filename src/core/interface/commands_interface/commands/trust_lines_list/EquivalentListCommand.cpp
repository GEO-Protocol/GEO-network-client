#include "EquivalentListCommand.h"

EquivalentListCommand::EquivalentListCommand(
    const CommandUUID &uuid,
    const string& commandBuffer)
    noexcept:

    BaseUserCommand(
        uuid,
        identifier())
{}

const string &EquivalentListCommand::identifier()
{
    static const string identifier = "GET:equivalents";
    return identifier;
}

CommandResult::SharedConst EquivalentListCommand::resultOk(
    string& equivalents) const
{
    return make_shared<const CommandResult>(
        identifier(),
        UUID(),
        200,
        equivalents);
}