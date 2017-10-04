#include "SetOutgoingTrustLineCommand.h"


SetOutgoingTrustLineCommand::SetOutgoingTrustLineCommand(
    const CommandUUID &commandUUID,
    const string &command) :

    BaseUserCommand(
        commandUUID,
        identifier())
{
    static const auto amountTokenOffset = NodeUUID::kHexSize + 1;
    static const auto minCommandLength = amountTokenOffset + 1;

    if (command.size() < minCommandLength) {
        throw ValueError(
            "SetTrustLineCommand: can't parse command. "
            "Received command is to short.");
    }

    try {
        string hexUUID = command.substr(0, NodeUUID::kHexSize);
        mContractorUUID = boost::lexical_cast<uuids::uuid>(hexUUID);

    } catch (...) {
        throw ValueError(
            "SetOutgoingTrustLineCommand: can't parse command. "
            "Error occurred while parsing 'Contractor UUID' token.");
    }

    try {
        for (size_t commandSeparatorPosition = amountTokenOffset; commandSeparatorPosition < command.length(); ++commandSeparatorPosition) {
            if (command.at(commandSeparatorPosition) == kCommandsSeparator) {
                mAmount = TrustLineAmount(
                    command.substr(
                        amountTokenOffset,
                        commandSeparatorPosition - amountTokenOffset));
            }
        }

    } catch (...) {
        throw ValueError(
            "SetTrustLineCommand: can't parse command. "
            "Error occurred while parsing 'New amount' token.");
    }
}

SetOutgoingTrustLineCommand::SetOutgoingTrustLineCommand(
    BytesShared buffer) :

    BaseUserCommand(identifier())
{
    // todo: move code from deserializeFromBytes here.
    deserializeFromBytes(buffer);
}

const string &SetOutgoingTrustLineCommand::identifier()
    noexcept
{
    static const string identifier = "SET:contractors/trust-lines";
    return identifier;
}

const NodeUUID &SetOutgoingTrustLineCommand::contractorUUID() const
    noexcept
{
    return mContractorUUID;
}

const TrustLineAmount &SetOutgoingTrustLineCommand::amount() const
    noexcept
{
    return mAmount;
}

pair<BytesShared, size_t> SetOutgoingTrustLineCommand::serializeToBytes() {

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
    vector<byte> buffer = trustLineAmountToBytes(mAmount);
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

void SetOutgoingTrustLineCommand::deserializeFromBytes(
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

    mAmount = bytesToTrustLineAmount(amountBytes);
}

const size_t SetOutgoingTrustLineCommand::kRequestedBufferSize()
    noexcept
{
    static const size_t size = kOffsetToInheritedBytes() + NodeUUID::kBytesSize + kTrustLineAmountBytesCount;
    return size;
}
