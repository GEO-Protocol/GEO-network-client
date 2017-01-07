#include "OpenTrustLineCommand.h"


OpenTrustLineCommand::OpenTrustLineCommand(
    const CommandUUID &uuid,
    const string &commandBuffer):

    BaseUserCommand(uuid, identifier()){

    deserialize(commandBuffer);
}

const string OpenTrustLineCommand::identifier() {
    static const string identifier = "CREATE:contractors/trust-lines";
    return identifier;
}

const NodeUUID& OpenTrustLineCommand::contractorUUID() const {
    return mContractorUUID;
}

const TrustLineAmount& OpenTrustLineCommand::amount() const {
    return mAmount;
}

const CommandResult *OpenTrustLineCommand::resultOk() const{
    return new CommandResult(uuid(), 200);
}

const CommandResult *OpenTrustLineCommand::trustLineAlreadyPresentResult() const{
    return new CommandResult(uuid(), 409);
}

/*!
 * Throws ValueError if deserialization was unsuccessful.
 */
void OpenTrustLineCommand::deserialize(
    const string &command) {

    const auto amountTokenOffset = NodeUUID::kHexSize + 1;
    const auto minCommandLength = amountTokenOffset + 1;

    if (command.size() < minCommandLength) {
        throw ValueError(
            "OpenTrustLineCommand::deserialize: "
                "Can't parse command. Received command is to short.");
    }


    try {
        string hexUUID = command.substr(0, NodeUUID::kHexSize);
        mContractorUUID = boost::lexical_cast<uuids::uuid>(hexUUID);

    } catch (...) {
        throw ValueError(
            "OpenTrustLineCommand::deserialize: "
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
            "OpenTrustLineCommand::deserialize: "
                "Can't parse command. Error occurred while parsing 'Amount' token.");
    }

    if (mAmount == TrustLineAmount(0)){
        throw ValueError(
            "OpenTrustLineCommand::deserialize: "
                "Can't parse command. Received 'Amount' can't be 0.");
    }
}

