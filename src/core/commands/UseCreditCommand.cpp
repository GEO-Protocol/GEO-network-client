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
    try {
        string hexUUID = mCommandBuffer.substr(0, kUUIDHexSize);
        NodeUUID uuid(boost::lexical_cast<uuids::uuid>(hexUUID));
        mContractorUUID = uuid;
    } catch (...) {
        throw CommandParsingError(
                "Can't parse command with 'CREATE:contractors/transations' identifier. Error occurred while parsing contractor 'UUID' token.");
    }

    const size_t amountOffset = kUUIDHexSize + 1;
    size_t purposeOffset = amountOffset;
    if (mCommandBuffer.size() > amountOffset) {
        string amount;
        for (size_t i = amountOffset; i < mCommandBuffer.size(); ++i) {
            char character = mCommandBuffer.at(i);
            if (character == kTokensSeparator || character == kCommandsSeparator) {
                break;
            }
            purposeOffset += 1;
            amount.push_back(character);
        }
        if (amount.size() > 0) {
            try {
                trust_amount value(amount);
                mAmount = value;
                if (mAmount == ZERO_TRUST_AMOUNT_VALUE) {
                    throw ConflictError("Can't parse command with 'CREATE:contractors/transations' identifier. 'Amount' token value must be greater than zero.");
                }
            } catch(...){
                throw ConflictError("Can't parse command with 'CREATE:contractors/transations' identifier. Can't cast token 'amount' value to boost::multiprecision.");
            }
        } else {
            throw CommandParsingError("Can't parse command with 'CREATE:contractors/transations' identifier. 'Amount' token must be greater than zero.");
        }
    } else {
        throw CommandParsingError("Can't parse command with 'CREATE:contractors/transations' identifier. 'Amount' token was not found.");
    }

    purposeOffset += 1;
    if (mCommandBuffer.size() > purposeOffset) {
        for (size_t i = purposeOffset; i < mCommandBuffer.size(); ++i) {
            char character = mCommandBuffer.at(i);
            if (character == kTokensSeparator || character == kCommandsSeparator) {
                break;
            }
            mPurpose.push_back(character);
        }
    }
}

