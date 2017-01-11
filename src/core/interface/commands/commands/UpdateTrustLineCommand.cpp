#include "UpdateTrustLineCommand.h"

UpdateTrustLineCommand::UpdateTrustLineCommand(
    const CommandUUID &uuid,
    const string &commandBuffer):

    BaseUserCommand(uuid, identifier()) {

    deserialize(commandBuffer);
}

const string &UpdateTrustLineCommand::identifier() {
    static const string identifier = "SET:contractors/trust-lines";
    return identifier;
}

const NodeUUID &UpdateTrustLineCommand::contractorUUID() const {
    return mContractorUUID;
}

const TrustLineAmount &UpdateTrustLineCommand::amount() const {
    return mAmount;
}

const CommandResult *UpdateTrustLineCommand::resultOk() const {
    return new CommandResult(uuid(), 200);
}

const CommandResult *UpdateTrustLineCommand::trustLineIsAbsentResult() const {
    return new CommandResult(uuid(), 404);
}

const CommandResult *UpdateTrustLineCommand::debtGreaterThanAmountResult() const {
    return new CommandResult(uuid(), 409);
}

const CommandResult *UpdateTrustLineCommand::resultConflict() const {
    return new CommandResult(uuid(), 429);
}

void UpdateTrustLineCommand::deserialize(
    const string &command) {

    const auto amountTokenOffset = CommandUUID::kUUIDLength + 1;
    const auto minCommandLength = amountTokenOffset + 1;

    if (command.size() < minCommandLength) {
        throw ValueError(
            "UpdateTrustLineCommand::deserialize: "
                "Can't parse command. Received command is to short.");
    }


    try {
        string hexUUID = command.substr(0, NodeUUID::kUUIDLength);
        mContractorUUID = boost::lexical_cast<uuids::uuid>(hexUUID);

    } catch (...) {
        throw ValueError(
            "UpdateTrustLineCommand::deserialize: "
                "Can't parse command. Error occurred while parsing 'Contractor UUID' token.");
    }


    try {
        for (size_t commandSeparatorPosition = amountTokenOffset; commandSeparatorPosition < command.length(); ++commandSeparatorPosition) {
            if (command.at(commandSeparatorPosition) == kCommandsSeparator) {
                mAmount = TrustLineAmount(command.substr(amountTokenOffset, commandSeparatorPosition - amountTokenOffset));
            }
        }

    } catch (...) {
        throw ValueError(
            "UpdateTrustLineCommand::deserialize: "
                "Can't parse command. Error occurred while parsing 'Amount' token.");
    }

    if (mAmount == TrustLineAmount(0)){
        throw ValueError(
            "UpdateTrustLineCommand::deserialize: "
                "Can't parse command. Received 'Amount' can't be 0.");
    }
}

