#include "GetFirstLevelContractorsBalancesCommand.h"

GetFirstLevelContractorsBalancesCommand::GetFirstLevelContractorsBalancesCommand(
    const CommandUUID &uuid,
    const string& commandBuffer)
    noexcept:
        BaseUserCommand(
                uuid,
                identifier())
{}

const string &GetFirstLevelContractorsBalancesCommand::identifier() {

    static const string identifier = "GET:contractors/trust-lines";
    return identifier;
}

CommandResult::SharedConst GetFirstLevelContractorsBalancesCommand::resultOk(string& neighbors) const {
    return make_shared<const CommandResult>(UUID(),200, neighbors);
}

void GetFirstLevelContractorsBalancesCommand::parse(const string& command)
{}
