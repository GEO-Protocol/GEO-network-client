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

const CommandResult *MaximalTransactionAmountCommand::resultOk(TrustLineAmount amount) const {
    string maximalAmount = boost::lexical_cast<string>(amount);
    return new CommandResult(uuid(), 200, maximalAmount);
}

void MaximalTransactionAmountCommand::deserialize(const string &command) {
    try {
        string hexUUID = command.substr(0, CommandUUID::kUUIDLength);
        mContractorUUID = boost::lexical_cast<uuids::uuid>(hexUUID);

    } catch (...) {
        throw ValueError(
            "MaximalTransactionAmountCommand::deserialize: "
                "Can't parse command. Error occurred while parsing 'Contractor UUID' token.");
    }
}




