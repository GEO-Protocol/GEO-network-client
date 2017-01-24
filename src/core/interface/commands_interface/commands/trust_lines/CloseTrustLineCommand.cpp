#include "CloseTrustLineCommand.h"

CloseTrustLineCommand::CloseTrustLineCommand(
    const CommandUUID &uuid,
    const string &commandBuffer):

    BaseUserCommand(
        uuid,
        identifier()
    ) {

    deserialize(commandBuffer);
}
const string &CloseTrustLineCommand::identifier() {

    static const string identifier = "REMOVE:contractors/trust-lines";
    return identifier;
}

const NodeUUID &CloseTrustLineCommand::contractorUUID() const {

    return mContractorUUID;

}

/**
 * Throws ValueError if deserialization was unsuccessful.
 */
void CloseTrustLineCommand::deserialize(const string &command) {

    try {
        string hexUUID = command.substr(0, CommandUUID::kHexSize);
        mContractorUUID = boost::lexical_cast<uuids::uuid>(hexUUID);

    } catch (...) {
        throw ValueError("CloseTrustLineCommand::deserialize: "
                             "Can't parse command. Error occurred while parsing 'Contractor UUID' token.");
    }
}

const CommandResult *CloseTrustLineCommand::resultOk() const {

    return new CommandResult(
        commandUUID(),
        200
    );
}
const CommandResult *CloseTrustLineCommand::trustLineIsAbsentResult() const {

    return new CommandResult(
        commandUUID(),
        404
    );
}

const CommandResult *CloseTrustLineCommand::resultConflict() const {

    return new CommandResult(
        commandUUID(),
        429
    );
}

const CommandResult *CloseTrustLineCommand::resultNoResponse() const {

    return new CommandResult(
        commandUUID(),
        444
    );
}

const CommandResult *CloseTrustLineCommand::resultTransactionConflict() const {

    return new CommandResult(
        commandUUID(),
        500
    );
}
