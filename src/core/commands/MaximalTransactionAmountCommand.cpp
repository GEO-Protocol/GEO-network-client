#include "MaximalTransactionAmountCommand.h"

MaximalTransactionAmountCommand::MaximalTransactionAmountCommand(const uuids::uuid &commandUUID,
                                                                 const string &identifier,
                                                                 const string &timestampExcepted,
                                                                 const string &commandBuffer) :
        Command(commandUUID, identifier, timestampExcepted) {
    mCommandBuffer = commandBuffer;
    deserialize();
}

const uuids::uuid &MaximalTransactionAmountCommand::commandUUID() const {
    return commandsUUID();
}

const string &MaximalTransactionAmountCommand::id() const {
    return identifier();
}

const string &MaximalTransactionAmountCommand::exceptedTimestamp() const {
    return timeStampExcepted();
}

const NodeUUID &MaximalTransactionAmountCommand::contractorUUID() const {
    return mContractorUUID;
}

void MaximalTransactionAmountCommand::deserialize() {
    try {
        string hexUUID = mCommandBuffer.substr(0, kUUIDHexSize);
        NodeUUID uuid(boost::lexical_cast<uuids::uuid>(hexUUID));
        mContractorUUID = uuid;
    } catch (...) {
        throw CommandParsingError("Can't parse command with 'GET:contractors/transations/max' identifier. Error occurred while parsing contractor 'UUID' token.");
    }
}

