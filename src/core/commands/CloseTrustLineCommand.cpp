CloseTrustLineCommand::CloseTrustLineCommand(const uuids::uuid &commandUUID, const string &identifier,
                                             const string &timestampExcepted, string &commandBuffer) :
        Command(identifier, timestampExcepted) {
    mCommandBuffer = commandBuffer;
    deserialize();
}

const uuids::uuid &CloseTrustLineCommand::commandUUID() const {
    commandsUUID();
}

const string &CloseTrustLineCommand::id() const {
    identifier();
}

const string &CloseTrustLineCommand::exceptedTimestamp() const {
    timeStampExcepted();
}

const uuids::uuid &CloseTrustLineCommand::contractorUUID() const {
    return mContractorUUID;
}

void OpenTrustLineCommand::deserialize() {
    string hexUUID = mCommandBuffer.substr(0, kUUIDHexSize);
    try {
        mContractorUUID = boost::lexical_cast<uuids::uuid>(hexUUID);
    } catch (...) {
        //TODO: create error for this exception type and throw
    }
}