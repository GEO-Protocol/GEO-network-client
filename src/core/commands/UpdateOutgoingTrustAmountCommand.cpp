#include "UpdateOutgoingTrustAmountCommand.h"

UpdateOutgoingTrustAmountCommand::UpdateOutgoingTrustAmountCommand(const uuids::uuid &commandUUID,
                                                                   const string &identifier,
                                                                   const string &timestampExcepted,
                                                                   const string &commandBuffer) :
        Command(commandUUID, identifier, timestampExcepted) {
    mCommandBuffer = commandBuffer;
    deserialize();
}

const uuids::uuid &UpdateOutgoingTrustAmountCommand::commandUUID() const {
    return commandsUUID();
}

const string &UpdateOutgoingTrustAmountCommand::id() const {
    return identifier();
}

const string &UpdateOutgoingTrustAmountCommand::exceptedTimestamp() const {
    return timeStampExcepted();
}

const NodeUUID &UpdateOutgoingTrustAmountCommand::contractorUUID() const {
    return mContractorUUID;
}

const trust_amount &UpdateOutgoingTrustAmountCommand::amount() const {
    return mAmount;
}

void UpdateOutgoingTrustAmountCommand::deserialize() {
    try {
        string hexUUID = mCommandBuffer.substr(0, kUUIDHexSize);
        NodeUUID uuid(boost::lexical_cast<uuids::uuid>(hexUUID));
        mContractorUUID = uuid;
    } catch (...) {
        throw CommandParsingError("Can't parse command with 'SET:contractors/trust-lines' identifier. Error occurred while parsing contractor 'UUID' token.");
    }

    const size_t amountOffset = kUUIDHexSize + 1;
    if (mCommandBuffer.size() > amountOffset) {
        string amount;
        for (size_t i = amountOffset; i < mCommandBuffer.size(); ++i) {
            char character = mCommandBuffer.at(i);
            if (character == kTokensSeparator || character == kCommandsSeparator) {
                break;
            }
            amount.push_back(character);
        }
        if (amount.size() > 0){
            try {
                trust_amount trustValue(amount);
                mAmount = trustValue;
                if (mAmount == ZERO_TRUST_AMOUNT_VALUE) {
                    throw ConflictError("Can't parse command with 'SET:contractors/trust-lines' identifier. 'Amount' token value must be greater than zero.");
                }
            } catch(...){
                throw ConflictError("Can't parse command with 'SET:contractors/trust-lines' identifier. Can't cast token 'amount' value to boost::multiprecision.");
            }
        } else {
            throw CommandParsingError("Can't parse command with 'SET:contractors/trust-lines' identifier. 'Amount' token must be greater than zero.");
        }
    } else {
        throw CommandParsingError("Can't parse command with 'SET:contractors/trust-lines' identifier. 'Amount' token was not found.");
    }
}

