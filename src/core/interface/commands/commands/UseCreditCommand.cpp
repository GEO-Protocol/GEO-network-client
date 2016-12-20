#include "UseCreditCommand.h"

UseCreditCommand::UseCreditCommand(
    const CommandUUID &uuid,
    const string &commandBuffer):

    BaseUserCommand(uuid, kIdentifier) {

    deserialize(commandBuffer);
}

const NodeUUID& UseCreditCommand::contractorUUID() const {
    return mContractorUUID;
}

const trust_amount &UseCreditCommand::amount() const {
    return mAmount;
}

const string &UseCreditCommand::purpose() const {
    return mPurpose;
}

void UseCreditCommand::deserialize(
    const string &command) {

    const auto amountTokenOffset = CommandUUID::kLength + 1;
    const auto minCommandLength = amountTokenOffset + 1;

    if (command.size() < minCommandLength) {
        throw ValueError(
            "UseCreditCommand::deserialize: "
                "Can't parse command. Received command is to short.");
    }


    try {
        string hexUUID = command.substr(0, NodeUUID::kUUIDLength);
        mContractorUUID = boost::lexical_cast<uuids::uuid>(hexUUID);

    } catch (...) {
        throw ValueError(
            "UseCreditCommand::deserialize: "
                "Can't parse command. Error occurred while parsing 'Contractor UUID' token.");
    }


    try {
        mAmount = trust_amount(command.substr(
            amountTokenOffset, command.find_last_of(kTokensSeparator)));
    } catch (...) {
        throw ValueError(
            "UseCreditCommand::deserialize: "
                "Can't parse command. Error occurred while parsing 'Amount' token.");
    }

    if (mAmount == trust_amount(0)){
        throw ValueError(
            "UseCreditCommand::deserialize: "
                "Can't parse command. Received 'Amount' can't be 0.");
    }


//    try {
//        mPurpose = command.substr(
//            amountTokenOffset, command.find_last_of(kTokensSeparator)));
//    } catch (...) {
//        throw ValueError(
//            "UseCreditCommand::deserialize: "
//                "Can't parse command. Error occurred while parsing 'Amount' token.");
//    }
//
//    if (mAmount == trust_amount(0)){
//        throw ValueError(
//            "UseCreditCommand::deserialize: "
//                "Can't parse command. Received 'Amount' can't be 0.");
//    }

}

