#include "TotalBalancesCommand.h"

TotalBalancesCommand::TotalBalancesCommand(
    const CommandUUID &uuid,
    const string &commandBuffer):

    BaseUserCommand(
        uuid,
        identifier()) {

    parse(commandBuffer);
}

const string &TotalBalancesCommand::identifier() {

    static const string identifier = "GET:/stats/balances/total";
    return identifier;
}

/**
 * Throws ValueError if deserialization was unsuccessful.
 */
void TotalBalancesCommand::parse(
        const string &command) {

    /*const auto amountTokenOffset = NodeUUID::kHexSize + 1;
    const auto minCommandLength = amountTokenOffset;

    if (command.size() < minCommandLength) {
        throw ValueError("InitiateMaxFlowCalculationCommand::parse: "
                                 "Can't parse command. Received command is to short.");
    }*/

}

CommandResult::SharedConst TotalBalancesCommand::resultOk(string &totalBalancesStr) const {

    return CommandResult::SharedConst(
        new CommandResult(
            UUID(),
            200,
            totalBalancesStr));
}
