#include "CyclesBaseFiveOrSixNodesBoundaryMessage.h"

CyclesBaseFiveOrSixNodesBoundaryMessage::CyclesBaseFiveOrSixNodesBoundaryMessage(
    const SerializedEquivalent equivalent,
    vector<BaseAddress::Shared>& path,
    vector<BaseAddress::Shared>& boundaryNodes) :

    CycleBaseFiveOrSixNodesInBetweenMessage(
        equivalent,
        // todo : this parameter is not useful, need message hierarchy changing
        0,
        path),
    mBoundaryNodes(boundaryNodes)
{}

CyclesBaseFiveOrSixNodesBoundaryMessage::CyclesBaseFiveOrSixNodesBoundaryMessage(
    BytesShared buffer):
    CycleBaseFiveOrSixNodesInBetweenMessage(
        buffer)
{
    size_t bytesBufferOffset = CycleBaseFiveOrSixNodesInBetweenMessage::kOffsetToInheritedBytes();
    //    Get NodesCount
    SerializedRecordsCount boundaryNodesCount;
    memcpy(
        &boundaryNodesCount,
        buffer.get() + bytesBufferOffset,
        sizeof(SerializedRecordsCount));
    bytesBufferOffset += sizeof(SerializedRecordsCount);
    //    Parse boundary nodes
    for (SerializedRecordNumber idx = 0; idx < boundaryNodesCount; idx++){
        auto stepAddress = deserializeAddress(
            buffer.get() + bytesBufferOffset);
        bytesBufferOffset += stepAddress->serializedSize();
        mBoundaryNodes.push_back(stepAddress);
    }
}

pair<BytesShared, size_t> CyclesBaseFiveOrSixNodesBoundaryMessage::serializeToBytes() const
{
    auto parentBytesAndCount = CycleBaseFiveOrSixNodesInBetweenMessage::serializeToBytes();

    auto boundaryNodesCount = (SerializedRecordsCount) mBoundaryNodes.size();
    size_t bytesCount =
        parentBytesAndCount.second +
        sizeof(SerializedRecordsCount);
    for (const auto &address : mBoundaryNodes) {
        bytesCount += address->serializedSize();
    }
    BytesShared dataBytesShared = tryCalloc(bytesCount);
    size_t dataBytesOffset = 0;
    // for parent node
    //----------------------------------------------------
    memcpy(
        dataBytesShared.get(),
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second);
    dataBytesOffset += parentBytesAndCount.second;
    //    for BoundaryNodes
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        &boundaryNodesCount,
        sizeof(SerializedRecordsCount));
    dataBytesOffset += sizeof(SerializedRecordsCount);

    for(const auto &address: mBoundaryNodes){
        auto serializedAddress = address->serializeToBytes();
        memcpy(
            dataBytesShared.get() + dataBytesOffset,
            serializedAddress.get(),
            address->serializedSize());
        dataBytesOffset += address->serializedSize();
    }

    return make_pair(
        dataBytesShared,
        bytesCount);
}

vector<BaseAddress::Shared> CyclesBaseFiveOrSixNodesBoundaryMessage::BoundaryNodes() const
{
    return mBoundaryNodes;
}