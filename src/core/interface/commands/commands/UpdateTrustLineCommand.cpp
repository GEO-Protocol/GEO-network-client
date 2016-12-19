#include "UpdateTrustLineCommand.h"

UpdateTrustLineCommand::UpdateTrustLineCommand(
    const CommandUUID &uuid,
    const string &commandBuffer):

    BaseUserCommand(uuid, kIdentifier) {

    deserialize(commandBuffer);
}

const NodeUUID &UpdateTrustLineCommand::contractorUUID() const {
    return mContractorUUID;
}

const trust_amount &UpdateTrustLineCommand::amount() const {
    return mAmount;
}

void UpdateTrustLineCommand::deserialize(
    const string &command) {

    const auto amountTokenOffset = CommandUUID::kLength + 1;
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
        mAmount = trust_amount(command.substr(amountTokenOffset));
    } catch (...) {
        throw ValueError(
            "UpdateTrustLineCommand::deserialize: "
                "Can't parse command. Error occurred while parsing 'Amount' token.");
    }

    if (mAmount == trust_amount(0)){
        throw ValueError(
            "UpdateTrustLineCommand::deserialize: "
                "Can't parse command. Received 'Amount' can't be 0.");
    }
}