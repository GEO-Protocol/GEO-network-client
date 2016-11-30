#include "CloseTrustLineCommand.h"

CloseTrustLineCommand::CloseTrustLineCommand(const uuids::uuid &commandUUID, const string &identifier,
                                             const string &timestampExcepted, const string &commandBuffer) :
        Command(identifier, timestampExcepted) {
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

const uuids::uuid &CloseTrustLineCommand::contractorUUID() const {
    return mContractorUUID;
}

void CloseTrustLineCommand::deserialize() {
    string hexUUID = mCommandBuffer.substr(0, kUUIDHexSize);
    try {
        mContractorUUID = boost::lexical_cast<uuids::uuid>(hexUUID);
    } catch (...) {
        //TODO: create error for this exception type and throw
    }
}