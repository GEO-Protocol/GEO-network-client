#include "ContractorListCommand.h"

ContractorListCommand::ContractorListCommand(
    const CommandUUID &uuid,
    const string &commandBuffer):

    BaseUserCommand(
        uuid,
        identifier())
{}

const string &ContractorListCommand::identifier()
{
    static const string identifier = "GET:contractors-all";
    return identifier;
}

CommandResult::SharedConst ContractorListCommand::resultOk(
    string &contractors) const
{
    return make_shared<const CommandResult>(
        identifier(),
        UUID(),
        200,
        contractors);
}