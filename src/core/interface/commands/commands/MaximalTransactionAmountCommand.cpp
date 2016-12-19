#include "MaximalTransactionAmountCommand.h"

MaximalTransactionAmountCommand::MaximalTransactionAmountCommand(
    const CommandUUID &uuid,
    const string &commandBuffer):

    BaseUserCommand(uuid, kIdentifier) {

    deserialize(commandBuffer);
}

const NodeUUID &MaximalTransactionAmountCommand::contractorUUID() const {
    return mContractorUUID;
}

void MaximalTransactionAmountCommand::deserialize(const string &command) {
    try {
        string hexUUID = command.substr(0, CommandUUID::kLength);
        mContractorUUID = boost::lexical_cast<uuids::uuid>(hexUUID);

    } catch (...) {
        throw ValueError(
            "MaximalTransactionAmountCommand::deserialize: "
                "Can't parse command. Error occurred while parsing 'Contractor UUID' token.");
    }
}

