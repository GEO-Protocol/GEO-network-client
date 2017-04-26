#include "FindPathCommand.h"

FindPathCommand::FindPathCommand(
    const CommandUUID &uuid,
    const string &commandBuffer):

    BaseUserCommand(
        uuid,
        identifier()) {

    parse(commandBuffer);
}

const string &FindPathCommand::identifier() {

    static const string identifier = "GET:/contractors/path";
    return identifier;
}

const NodeUUID &FindPathCommand::contractorUUID() const {

    return mContractorUUID;
}

/**
 * Throws ValueError if deserialization was unsuccessful.
 */
void FindPathCommand::parse(
        const string &command) {

    const auto amountTokenOffset = NodeUUID::kHexSize + 1;
    const auto minCommandLength = amountTokenOffset;

    if (command.size() < minCommandLength) {
        throw ValueError("FindPathCommand::parse: "
                                 "Can't parse command. Received command is to short.");
    }

    try {
        string hexUUID = command.substr(
            0,
            NodeUUID::kHexSize);
        mContractorUUID = boost::lexical_cast<uuids::uuid>(hexUUID);

    } catch (...) {
        throw ValueError("FindPathCommand::parse: "
                                 "Can't parse command. Error occurred while parsing 'Contractor UUID' token.");
    }

}

CommandResult::SharedConst FindPathCommand::resultOk(string &path) const {

    return CommandResult::SharedConst(
        new CommandResult(
            UUID(),
            200,
            path));
}