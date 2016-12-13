#include "UseCreditCommand.h"

UseCreditCommand::UseCreditCommand(const uuids::uuid &commandUUID, const string &identifier,
                                   const string &timestampExcepted, const string &commandBuffer) :
        Command(commandUUID, identifier, timestampExcepted) {
    mCommandBuffer = commandBuffer;
    deserialize();
}

const uuids::uuid &UseCreditCommand::commandUUID() const {
    return commandsUUID();
}

const string &UseCreditCommand::id() const {
    return identifier();
}

const string &UseCreditCommand::exceptedTimestamp() const {
    return timeStampExcepted();
}

const NodeUUID &UseCreditCommand::contractorUUID() const {
    return mContractorUUID;
}

const trust_amount &UseCreditCommand::amount() const {
    return mAmount;
}

const string &UseCreditCommand::purpose() const {
    return mPurpose;
}

void UseCreditCommand::deserialize() {
    string hexUUID = mCommandBuffer.substr(0, kUUIDHexSize);
    try {
        NodeUUID uuid(boost::lexical_cast<uuids::uuid>(hexUUID));
        mContractorUUID = uuid;
    } catch (...) {
        //TODO: create error for this exception type and throw
    }

    const size_t amountOffset = kUUIDHexSize + 1;
    size_t purposeOffset = amountOffset;
    string amount;
    for (size_t i = amountOffset; i < mCommandBuffer.size(); ++i) {
        char character = mCommandBuffer.at(i);
        if (character == kTokensSeparator || character == kCommandsSeparator) {
            break;
        }
        purposeOffset += 1;
        amount.push_back(character);
    }
    if (amount.size() > 0){
        trust_amount value(amount);
        mAmount = value;
    } else {
        //TODO:: throw error
    }

    purposeOffset +=1;
    mPurpose = "";
    for (size_t i = purposeOffset; i < mCommandBuffer.size(); ++i){
        char character = mCommandBuffer.at(i);
        if (character == kTokensSeparator || character == kCommandsSeparator){
            break;
        }
        mPurpose.push_back(character);
    }
}

