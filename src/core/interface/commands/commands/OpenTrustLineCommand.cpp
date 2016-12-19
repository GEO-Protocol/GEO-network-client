#include "OpenTrustLineCommand.h"


OpenTrustLineCommand::OpenTrustLineCommand(
    const CommandUUID &uuid,
    const string &commandBuffer):

    BaseUserCommand(uuid, kIdentifier){

    deserialize(commandBuffer);
}

const NodeUUID& OpenTrustLineCommand::contractorUUID() const {
    return mContractorUUID;
}

const trust_amount& OpenTrustLineCommand::amount() const {
    return mAmount;
}

/*!
 * Throws ValueError if deserialization was unsuccessful.
 */
void OpenTrustLineCommand::deserialize(
    const string &command) {

    const auto amountTokenOffset = CommandUUID::kLength + 1;
    const auto minCommandLength = amountTokenOffset + 1;

    if (command.size() < minCommandLength) {
        throw ValueError(
            "OpenTrustLineCommand::deserialize: "
                "Can't parse command. Received command is to short.");
    }


    try {
        string hexUUID = command.substr(0, NodeUUID::kUUIDLength);
        mContractorUUID = boost::lexical_cast<uuids::uuid>(hexUUID);

    } catch (...) {
        throw ValueError(
            "OpenTrustLineCommand::deserialize: "
                "Can't parse command. Error occurred while parsing 'Contractor UUID' token.");
    }


    try {
        mAmount = trust_amount(command.substr(amountTokenOffset));
    } catch (...) {
        throw ValueError(
            "OpenTrustLineCommand::deserialize: "
                "Can't parse command. Error occurred while parsing 'Amount' token.");
    }

    if (mAmount == trust_amount(0)){
        throw ValueError(
            "OpenTrustLineCommand::deserialize: "
                "Can't parse command. Received 'Amount' can't be 0.");
    }
}
