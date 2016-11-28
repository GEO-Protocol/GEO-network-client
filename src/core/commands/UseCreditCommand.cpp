#include "UseCreditCommand.h"

UseCreditCommand::UseCreditCommand(const uuids::uuid &commandUUID, const string &identifier,
                                   const string &timestampExcepted, string &commandBuffer) :
        Command(identifier, timestampExcepted) {
    mCommandBuffer = commandBuffer;
    deserialize();
}

const uuids::uuid &UseCreditCommand::commandUUID() const {
    commandsUUID();
}

const string &UseCreditCommand::id() const {
    identifier();
}

const string &UseCreditCommand::exceptedTimestamp() const {
    timeStampExcepted();
}

const uuids::uuid &UseCreditCommand::contractorUUID() const {
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
        mContractorUUID = boost::lexical_cast<uuids::uuid>(hexUUID);
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
        amount.push_back(symbol);
    }
    if (amount.size() > 0){
        mAmount(amount);
    } else {
        //TODO:: throw error
    }

    purposeOffset +=1;

    mPurpose = mCommandBuffer.substr(purposeOffset, mCommandBuffer.size());
}

