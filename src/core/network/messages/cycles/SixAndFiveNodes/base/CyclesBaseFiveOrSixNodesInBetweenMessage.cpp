#include "CyclesBaseFiveOrSixNodesInBetweenMessage.h"

CycleBaseFiveOrSixNodesInBetweenMessage::CycleBaseFiveOrSixNodesInBetweenMessage(
    const SerializedEquivalent equivalent,
    ContractorID idOnReceiverSide,
    vector<BaseAddress::Shared> &path):
    SenderMessage(
        equivalent,
        idOnReceiverSide),
    mPath(path)
{}

CycleBaseFiveOrSixNodesInBetweenMessage::CycleBaseFiveOrSixNodesInBetweenMessage(
    BytesShared buffer):
    SenderMessage(buffer)
{
    size_t bytesBufferOffset = SenderMessage::kOffsetToInheritedBytes();

    // path
    SerializedPositionInPath nodesInPath;
    memcpy(
        &nodesInPath,
        buffer.get() + bytesBufferOffset,
        sizeof(SerializedPathLengthSize));
    bytesBufferOffset += sizeof(SerializedPathLengthSize);

    for (SerializedPositionInPath idx = 0; idx < nodesInPath; idx++) {
        auto stepAddress = deserializeAddress(
            buffer.get() + bytesBufferOffset);
        bytesBufferOffset += stepAddress->serializedSize();
        mPath.push_back(stepAddress);
    }
}

pair<BytesShared, size_t> CycleBaseFiveOrSixNodesInBetweenMessage::serializeToBytes() const
{
    auto parentBytesAndCount = SenderMessage::serializeToBytes();
    auto kNodesInPath = (SerializedPathLengthSize)mPath.size();
    size_t bytesCount = parentBytesAndCount.second
                        + sizeof(SerializedPathLengthSize);
    for (const auto &address : mPath) {
        bytesCount += address->serializedSize();
    }
    BytesShared dataBytesShared = tryCalloc(bytesCount);
    size_t dataBytesOffset = 0;
    //----------------------------------------------------
    memcpy(
        dataBytesShared.get(),
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second);
    dataBytesOffset += parentBytesAndCount.second;
    // For path
    // Write vector size first
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        &kNodesInPath,
        sizeof(SerializedPathLengthSize));
    dataBytesOffset += sizeof(kNodesInPath);

    for(auto const &address: mPath) {
        auto serializedAddress = address->serializeToBytes();
        memcpy(
            dataBytesShared.get() + dataBytesOffset,
            serializedAddress.get(),
            address->serializedSize());
        dataBytesOffset += address->serializedSize();
    }
    //----------------------------------------------------
    return make_pair(
        dataBytesShared,
        bytesCount);
}

const size_t CycleBaseFiveOrSixNodesInBetweenMessage::kOffsetToInheritedBytes() const
{
    auto kNodesInPath = (SerializedPathLengthSize)mPath.size();
    size_t offset = SenderMessage::kOffsetToInheritedBytes()
            + sizeof(SerializedPathLengthSize);
    for (const auto &address : mPath) {
        offset += address->serializedSize();
    }
    return offset;
}


vector<BaseAddress::Shared> CycleBaseFiveOrSixNodesInBetweenMessage::path() const
{
    return mPath;
}
