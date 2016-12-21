#include "CloseTrustLineCommand.h"


CloseTrustLineCommand::CloseTrustLineCommand(
    const CommandUUID &uuid,
    const string &commandBuffer):

    BaseUserCommand(uuid, identifier()) {

    deserialize(commandBuffer);
}

const string &CloseTrustLineCommand::identifier() {
    static const string identifier = "REMOVE:contractors/trust-lines";
    return identifier;
}

const NodeUUID& CloseTrustLineCommand::contractorUUID() const {
    return mContractorUUID;
}

const CommandResult *CloseTrustLineCommand::resultOk() const {
    return new CommandResult(uuid(), 200);
}

const CommandResult *CloseTrustLineCommand::trustLineIsAbsentResult() const {
    return new CommandResult(uuid(), 404);
}

const CommandResult *CloseTrustLineCommand::trustLineIsAbsentOrInvalidResult() const {
    return new CommandResult(uuid(), 409);
}

/*!
 * Throws ValueError if deserialization was unsuccessful.
 */
void CloseTrustLineCommand::deserialize(const string &command) {
    try {
        string hexUUID = command.substr(0, CommandUUID::kUUIDLength);
        mContractorUUID = boost::lexical_cast<uuids::uuid>(hexUUID);

    } catch (...) {
        throw ValueError(
            "CloseTrustLineCommand::deserialize: "
                "Can't parse command. Error occurred while parsing 'Contractor UUID' token.");
    }
}

