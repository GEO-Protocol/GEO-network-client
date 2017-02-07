#include "CreditUsageCommand.h"

CreditUsageCommand::CreditUsageCommand(
    const CommandUUID &uuid,
    const string &commandBuffer) :

    BaseUserCommand(
        uuid,
        identifier()) {

    deserialize(commandBuffer);
}

CreditUsageCommand::CreditUsageCommand(
    BytesShared buffer) {

    deserializeFromBytes(buffer);
}

const string &CreditUsageCommand::identifier() {
    static const string identifier = "CREATE:contractors/transactions";
    return identifier;
}

const NodeUUID &CreditUsageCommand::contractorUUID() const {
    return mContractorUUID;
}

const TrustLineAmount &CreditUsageCommand::amount() const {
    return mAmount;
}

const string &CreditUsageCommand::reason() const {
    return mReason;
}

/*!
 *
 *
 * Throws bad_alloc;
 */
pair<BytesShared, size_t> CreditUsageCommand::serializeToBytes() {

    auto buffer = tryMalloc(kRequestedBufferSize());

    // Contractor UUID serialization
    memcpy(buffer.get(), mContractorUUID.data, NodeUUID::kBytesSize);

    // Trust line amount serialization
    vector<byte> amountBytes;
    amountBytes.reserve(
        kTrustLineAmountBytesCount);

    export_bits(
        mAmount,
        back_inserter(amountBytes),
        8);

    // Filling amount with trailing zeroes.
    auto unusedAmountPlace = kTrustLineAmountBytesCount - amountBytes.size();
    for (size_t i = 0; i < unusedAmountPlace; ++i) {
        amountBytes.push_back(0);
    }

    memcpy(
        buffer.get() + NodeUUID::kBytesSize,
        amountBytes.data(),
        kTrustLineAmountBytesCount);


    return pair<BytesShared, size_t>();
}

void CreditUsageCommand::deserialize(
    const string &command) {

    const auto minCommandLength = CommandUUID::kHexSize + 1;
    if (command.size() < minCommandLength) {
        throw ValueError(
            "CreditUsageCommand::deserialize: "
                "can't parse command. Received command is too short.");
    }

    try {
        string hexUUID = command.substr(0, CommandUUID::kHexSize);
        mContractorUUID = boost::lexical_cast<uuids::uuid>(hexUUID);

    } catch (...) {
        throw ValueError(
            "CreditUsageCommand::deserialize: "
                "Can't parse command. Error occurred while parsing 'Contractor UUID' token.");
    }


    size_t purposeTokenStartPosition;
    for (size_t i = NodeUUID::kHexSize+1; i < command.length(); ++i) {
        if (command.at(i) == kTokensSeparator ||
            command.at(i) == kCommandsSeparator ||
            i == command.length()-1) {

            try {
                mAmount = TrustLineAmount(
                    command.substr(
                        NodeUUID::kHexSize+1,
                        i - NodeUUID::kHexSize-1));

            } catch (...) {
                throw ValueError(
                    "CreditUsageCommand::deserialize: "
                        "Can't parse command. Error occurred while parsing 'Amount' token.");
            }

            if (mAmount == TrustLineAmount(0)) {
                throw ValueError(
                    "CreditUsageCommand::deserialize: "
                        "Received 'Amount' can't be 0.");
            }

            purposeTokenStartPosition = i + 1;
            break;
        }
    }

    // todo: (hsc) add purpose parsing
}

const size_t CreditUsageCommand::kRequestedBufferSize() {
    return NodeUUID::kBytesSize + kTrustLineAmountBytesCount;
}

void CreditUsageCommand::deserializeFromBytes(BytesShared buffer) {
    throw ValueError("Not implemented.");
}

const CommandResult::Shared CreditUsageCommand::resultOk() const {
    return make_shared<CommandResult>(
        mCommandUUID,
        CommandResult::OK);
}
