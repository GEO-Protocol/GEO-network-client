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
    return make_shared<const CommandResult>(
        identifier(),
        UUID(),
        200,
        neighbors);
}
