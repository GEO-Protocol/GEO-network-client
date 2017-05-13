#include "GetFirstLevelContractorsCommand.h"

GetFirstLevelContractorsCommand::GetFirstLevelContractorsCommand(const CommandUUID &uuid, const string& commandBuffer)
    noexcept
    : BaseUserCommand(
    uuid,
    identifier())
{}

const string &GetFirstLevelContractorsCommand::identifier() {

    static const string identifier = "GET:contractors";
    return identifier;
}

CommandResult::SharedConst GetFirstLevelContractorsCommand::resultOk(string& neighbors) const {
    return make_shared<const CommandResult>(UUID(),200, neighbors);
}

void GetFirstLevelContractorsCommand::parse(const string& command)
{}
