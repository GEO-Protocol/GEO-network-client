#include "CloseTrustLineCommand.h"

CloseTrustLineCommand::CloseTrustLineCommand(const uuids::uuid &commandUUID, const string &identifier,
                                             const string &timestampExcepted, const string &commandBuffer) :
        Command(commandUUID, identifier, timestampExcepted) {
    mCommandBuffer = commandBuffer;
    deserialize();
}

const uuids::uuid &CloseTrustLineCommand::commandUUID() const {
    return commandsUUID();
}

const string &CloseTrustLineCommand::id() const {
    return identifier();
}

const string &CloseTrustLineCommand::exceptedTimestamp() const {
    return timeStampExcepted();
}

const NodeUUID &CloseTrustLineCommand::contractorUUID() const {
    return mContractorUUID;
}

void CloseTrustLineCommand::deserialize() {
    string hexUUID = mCommandBuffer.substr(0, kUUIDHexSize);
    try {
        NodeUUID uuid(boost::lexical_cast<uuids::uuid>(hexUUID));
        mContractorUUID = uuid;
    } catch (...) {
        //TODO: create error for this exception type and throw
    }
}