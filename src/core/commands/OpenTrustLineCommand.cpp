#include "OpenTrustLineCommand.h"

OpenTrustLineCommand::OpenTrustLineCommand(const uuids::uuid &commandUUID, const string &identifier,
                                           const string &timestampExcepted, const string &commandBuffer) :
        Command(commandUUID, identifier, timestampExcepted) {
    mCommandBuffer = commandBuffer;
    deserialize();
}

const uuids::uuid &OpenTrustLineCommand::commandUUID() const {
    return commandsUUID();
}

const string &OpenTrustLineCommand::id() const {
    return identifier();
}

const string &OpenTrustLineCommand::exceptedTimestamp() const {
    return timeStampExcepted();
}

const NodeUUID &OpenTrustLineCommand::contractorUUID() const {
    return mContractorUUID;
}

const trust_amount &OpenTrustLineCommand::amount() const {
    return mAmount;
}

void OpenTrustLineCommand::deserialize() {
    string hexUUID = mCommandBuffer.substr(0, kUUIDHexSize);
    try {
        NodeUUID uuid(boost::lexical_cast<uuids::uuid>(hexUUID));
        mContractorUUID = uuid;
    } catch (...) {
        //TODO: create error for this exception type and throw
    }

    const size_t amountOffset = kUUIDHexSize + 1;
    string amount;
    for (size_t i = amountOffset; i < mCommandBuffer.size(); ++i) {
        char character = mCommandBuffer.at(i);
        if (character == kTokensSeparator || character == kCommandsSeparator) {
            break;
        }
        amount.push_back(character);
    }
    if (amount.size() > 0){
        trust_amount trustValue(amount);
        mAmount = trustValue;
    }
}

