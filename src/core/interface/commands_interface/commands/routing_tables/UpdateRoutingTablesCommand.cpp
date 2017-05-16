#include "UpdateRoutingTablesCommand.h"

UpdateRoutingTablesCommand::UpdateRoutingTablesCommand(
    const CommandUUID &uuid,
    const string &commandBuffer)
noexcept:
    BaseUserCommand(
        uuid,
        identifier())
{}

const string &UpdateRoutingTablesCommand::identifier()
{
    static const string kIdentifier = "POST:routing_tables/update";
    return kIdentifier;
}


void UpdateRoutingTablesCommand::parse(const string &command)
// This command does not requires any parameters. Command UUID and Identifier are parsed separately
{}