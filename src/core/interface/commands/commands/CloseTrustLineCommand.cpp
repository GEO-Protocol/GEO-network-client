#include "CloseTrustLineCommand.h"


CloseTrustLineCommand::CloseTrustLineCommand(
    const CommandUUID &uuid,
    const string &commandBuffer):

    BaseUserCommand(uuid, identifier()) {

    deserialize(commandBuffer);
}

const NodeUUID& CloseTrustLineCommand::contractorUUID() const {
    return mContractorUUID;
}

const string &CloseTrustLineCommand::identifier() {
    static const string identifier = "REMOVE:contractors/trust-lines";
    return identifier;
}

/*!
 * Throws ValueError if deserialization was unsuccessful.
 */
void CloseTrustLineCommand::deserialize(const string &command) {
    try {
        string hexUUID = command.substr(0, CommandUUID::kLength);
        mContractorUUID = boost::lexical_cast<uuids::uuid>(hexUUID);

    } catch (...) {
        throw ValueError(
            "CloseTrustLineCommand::deserialize: "
                "Can't parse command. Error occurred while parsing 'Contractor UUID' token.");
    }
}
