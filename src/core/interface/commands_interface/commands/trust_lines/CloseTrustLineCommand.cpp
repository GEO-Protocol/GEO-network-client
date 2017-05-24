#include "CloseTrustLineCommand.h"

CloseTrustLineCommand::CloseTrustLineCommand(
    const CommandUUID &uuid,
    const string &commandBuffer):

    BaseUserCommand(
        uuid,
        identifier()
    ) {

    parse(commandBuffer);
}

CloseTrustLineCommand::CloseTrustLineCommand(
    BytesShared buffer):

    BaseUserCommand(identifier()) {

    deserializeFromBytes(buffer);
}

const string &CloseTrustLineCommand::identifier() {

    static const string identifier = "CLOSE:contractors/trust-lines";
    return identifier;
}

const NodeUUID &CloseTrustLineCommand::contractorUUID() const {

    return mContractorUUID;

}

pair<BytesShared, size_t> CloseTrustLineCommand::serializeToBytes() {

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
    //----------------------------------------------------
    return make_pair(
        dataBytesShared,
        bytesCount
    );
}

void CloseTrustLineCommand::deserializeFromBytes(
    BytesShared buffer) {

    BaseUserCommand::deserializeFromBytes(buffer);
    size_t bytesBufferOffset = BaseUserCommand::kOffsetToInheritedBytes();
    //----------------------------------------------------
    memcpy(
        mContractorUUID.data,
        buffer.get() + bytesBufferOffset,
        NodeUUID::kBytesSize
    );
}

/**
 * Throws ValueError if deserialization was unsuccessful.
 */
void CloseTrustLineCommand::parse(
    const string &command) {

    try {
        string hexUUID = command.substr(
            0,
            CommandUUID::kHexSize
        );
        mContractorUUID = boost::lexical_cast<uuids::uuid>(hexUUID);
    } catch (...) {
        throw ValueError("CloseTrustLineCommand::parse: "
                             "Can't parse command. Error occurred while parsing 'Contractor UUID' token.");
    }
}

const size_t CloseTrustLineCommand::kRequestedBufferSize() {

    static const size_t size = kOffsetToInheritedBytes() + NodeUUID::kBytesSize;
    return size;
}


CommandResult::SharedConst CloseTrustLineCommand::responseOK(uint16_t code) const
noexcept
{
    return makeResult(code);
}