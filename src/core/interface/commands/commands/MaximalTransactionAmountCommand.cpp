#include "MaximalTransactionAmountCommand.h"

MaximalTransactionAmountCommand::MaximalTransactionAmountCommand(
    const CommandUUID &uuid,
    const string &commandBuffer):

    BaseUserCommand(uuid, identifier()) {

    deserialize(commandBuffer);
}

const string &MaximalTransactionAmountCommand::identifier() {
    static const string identifier = "GET:contractors/transactions/max";
    return identifier;
}

const NodeUUID &MaximalTransactionAmountCommand::contractorUUID() const {
    return mContractorUUID;
}

const trust_amount &MaximalTransactionAmountCommand::amount() const {
    return mAmount;
}

const CommandResult *MaximalTransactionAmountCommand::resultOk() const {
    string maximalAmount = boost::lexical_cast<string>(mAmount);
    return new CommandResult(uuid(), 200, maximalAmount);
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




