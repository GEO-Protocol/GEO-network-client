#include "SetTrustLineCommand.h"

SetTrustLineCommand::SetTrustLineCommand(
    const CommandUUID &commandUUID,
    const string &commandBuffer) :

    BaseUserCommand(
    commandUUID,
    identifier()
    ) {

    parse(commandBuffer);
}

SetTrustLineCommand::SetTrustLineCommand(
    BytesShared buffer) :

    BaseUserCommand(identifier()) {

    deserializeFromBytes(buffer);
}

const string &SetTrustLineCommand::identifier() {

    static const string identifier = "SET:contractors/trust-lines";
    return identifier;
}

const NodeUUID &SetTrustLineCommand::contractorUUID() const {

    return mContractorUUID;
}

const TrustLineAmount &SetTrustLineCommand::newAmount() const {

    return mNewAmount;
}

pair<BytesShared, size_t> SetTrustLineCommand::serializeToBytes() {

    auto parentBytesAndCount = BaseUserCommand::serializeToBytes();

    size_t bytesCount = parentBytesAndCount.second +
        NodeUUID::kBytesSize +
        kTrustLineAmountBytesCount;
    BytesShared dataBytesShared = tryCalloc(bytesCount);
    size_t dataBytesOffset = 0;
    //----------------------------------------------------
    memcpy(
        dataBytesShared.get(),
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second
    );
    dataBytesOffset += parentBytesAndCount.second;
    //----------------------------------------------------
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        mContractorUUID.data,
        NodeUUID::kBytesSize
    );
    dataBytesOffset += NodeUUID::kBytesSize;
    //----------------------------------------------------
    vector<byte> buffer = trustLineAmountToBytes(mNewAmount);
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        buffer.data(),
        buffer.size()
    );
    //----------------------------------------------------
    return make_pair(
        dataBytesShared,
        bytesCount
    );
}

void SetTrustLineCommand::deserializeFromBytes(
    BytesShared buffer) {

    BaseUserCommand::deserializeFromBytes(buffer);
    size_t bytesBufferOffset = BaseUserCommand::kOffsetToInheritedBytes();
    //----------------------------------------------------
    memcpy(
        mContractorUUID.data,
        buffer.get() + bytesBufferOffset,
        NodeUUID::kBytesSize
    );
    bytesBufferOffset += NodeUUID::kBytesSize;
    //----------------------------------------------------
    vector<byte> amountBytes(
        buffer.get() + bytesBufferOffset,
        buffer.get() + bytesBufferOffset + kTrustLineAmountBytesCount);

    mNewAmount = bytesToTrustLineAmount(amountBytes);
}

void SetTrustLineCommand::parse(
    const string &command) {

    const auto amountTokenOffset = NodeUUID::kHexSize + 1;
    const auto minCommandLength = amountTokenOffset + 1;

    if (command.size() < minCommandLength) {
        throw ValueError("SetTrustLineCommand::parse: "
                             "Can't parse command. Received command is to short.");
    }

    try {
        string hexUUID = command.substr(
            0,
            NodeUUID::kHexSize
        );
        mContractorUUID = boost::lexical_cast<uuids::uuid>(hexUUID);

    } catch (...) {
        throw ValueError("SetTrustLineCommand::parse: "
                             "Can't parse command. Error occurred while parsing 'Contractor UUID' token.");
    }

    try {
        for (size_t commandSeparatorPosition = amountTokenOffset; commandSeparatorPosition < command.length(); ++commandSeparatorPosition) {
            if (command.at(commandSeparatorPosition) == kCommandsSeparator) {
                mNewAmount = TrustLineAmount(
                    command.substr(
                        amountTokenOffset,
                        commandSeparatorPosition - amountTokenOffset
                    )
                );
            }
        }

    } catch (...) {
        throw ValueError("SetTrustLineCommand::parse: "
                             "Can't parse command. Error occurred while parsing 'New amount' token.");
    }

    if (mNewAmount == TrustLineAmount(0)){
        throw ValueError("SetTrustLineCommand::parse: "
                             "Can't parse command. Received 'New amount' can't be 0.");
    }
}

const size_t SetTrustLineCommand::kRequestedBufferSize() {

    static const size_t size = kOffsetToInheritedBytes() + NodeUUID::kBytesSize + kTrustLineAmountBytesCount;
    return size;
}