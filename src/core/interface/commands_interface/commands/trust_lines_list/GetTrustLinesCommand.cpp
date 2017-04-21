#include "GetTrustLinesCommand.h"

GetTrustLinesCommand::GetTrustLinesCommand(
    const CommandUUID &uuid,
    const string &commandBuffer)
    noexcept:
        BaseUserCommand(
            uuid,
            identifier())
{}

const string &GetTrustLinesCommand::identifier()
{
    static const string kIdentifier = "GET:contractors/trust-lines";
    return kIdentifier;
}

CommandResult::SharedConst GetTrustLinesCommand::resultOk(
    string &neighbors) const
{
    return make_shared<const CommandResult>(UUID(),200, neighbors);
}

void GetTrustLinesCommand::parse(const string &command)
// This command does not requires any parameters. Command UUID and Identifier are parsed separately
{}
