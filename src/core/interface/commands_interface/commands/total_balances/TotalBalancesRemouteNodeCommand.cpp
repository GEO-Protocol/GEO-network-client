#include "TotalBalancesRemouteNodeCommand.h"

TotalBalancesRemouteNodeCommand::TotalBalancesRemouteNodeCommand(
    const CommandUUID &uuid,
    const string &commandBuffer):

    BaseUserCommand(
        uuid,
        identifier()) {

    parse(commandBuffer);
}

const string &TotalBalancesRemouteNodeCommand::identifier() {

    static const string identifier = "GET:nodes/stats/balances/total";
    return identifier;
}

const NodeUUID &TotalBalancesRemouteNodeCommand::contractorUUID() const {

    return mContractorUUID;
}

/**
 * Throws ValueError if deserialization was unsuccessful.
 */
void TotalBalancesRemouteNodeCommand::parse(
        const string &command) {

    const auto amountTokenOffset = NodeUUID::kHexSize + 1;
    const auto minCommandLength = amountTokenOffset;

    if (command.size() < minCommandLength) {
        throw ValueError("TotalBalancesRemouteNodeCommand::parse: "
                                 "Can't parse command. Received command is to short.");
    }

    try {
        string hexUUID = command.substr(
                0,
                NodeUUID::kHexSize);
        mContractorUUID = boost::lexical_cast<uuids::uuid>(hexUUID);

    } catch (...) {
        throw ValueError("TotalBalancesRemouteNodeCommand::parse: "
                                 "Can't parse command. Error occurred while parsing 'Contractor UUID' token.");
    }
}

CommandResult::SharedConst TotalBalancesRemouteNodeCommand::resultOk(string &totalBalancesStr) const {

    return CommandResult::SharedConst(
        new CommandResult(
            UUID(),
            200,
            totalBalancesStr));
}

CommandResult::SharedConst TotalBalancesRemouteNodeCommand::resultNoResponse() const {

    return CommandResult::SharedConst(
        new CommandResult(
            UUID(),
            501));
}
