#include "SetTrustLineCommand.h"

SetTrustLineCommand::SetTrustLineCommand(
    const CommandUUID &commandUUID,
    const string &commandBuffer) :

    BaseUserCommand(
    commandUUID,
    commandIdentifier()
    ) {

    deserialize(commandBuffer);
}

const string &SetTrustLineCommand::identifier() {

    static const string identifier = "SET:contractors/trust-lines";
    return identifier;
}

const NodeUUID &SetTrustLineCommand::contractorUUID() const {

    return mContractorUUID;
}

const TrustLineAmount &SetTrustLineCommand::newAmount() const {

    return mNewAmount;
}

void SetTrustLineCommand::deserialize(
    const string &command) {

    const auto amountTokenOffset = NodeUUID::kHexSize + 1;
    const auto minCommandLength = amountTokenOffset + 1;

    if (command.size() < minCommandLength) {
        throw ValueError("SetTrustLineCommand::deserialize: "
                             "Can't parse command. Received command is to short.");
    }

    try {
        string hexUUID = command.substr(0, NodeUUID::kHexSize);
        mContractorUUID = boost::lexical_cast<uuids::uuid>(hexUUID);

    } catch (...) {
        throw ValueError("SetTrustLineCommand::deserialize: "
                             "Can't parse command. Error occurred while parsing 'Contractor UUID' token.");
    }

    try {
        for (size_t commandSeparatorPosition = amountTokenOffset; commandSeparatorPosition < command.length(); ++commandSeparatorPosition) {
            if (command.at(commandSeparatorPosition) == kCommandsSeparator) {
                mNewAmount = TrustLineAmount(command.substr(amountTokenOffset, commandSeparatorPosition - amountTokenOffset));
            }
        }

    } catch (...) {
        throw ValueError("SetTrustLineCommand::deserialize: "
                             "Can't parse command. Error occurred while parsing 'New amount' token.");
    }

    if (mNewAmount == TrustLineAmount(0)){
        throw ValueError("SetTrustLineCommand::deserialize: "
                             "Can't parse command. Received 'New amount' can't be 0.");
    }
}

const CommandResult *SetTrustLineCommand::resultOk() const {

    return new CommandResult(
        commandUUID(),
        200
    );
}

const CommandResult *SetTrustLineCommand::trustLineAbsentResult() const {

    return new CommandResult(
        commandUUID(),
        404
    );
}

const CommandResult *SetTrustLineCommand::resultConflict() const {

    return new CommandResult(
        commandUUID(),
        429
    );
}

const CommandResult *SetTrustLineCommand::resultNoResponse() const {

    return new CommandResult(
        commandUUID(),
        444
    );
}

const CommandResult *SetTrustLineCommand::resultTransactionConflict() const {

    return new CommandResult(
        commandUUID(),
        500
    );
}