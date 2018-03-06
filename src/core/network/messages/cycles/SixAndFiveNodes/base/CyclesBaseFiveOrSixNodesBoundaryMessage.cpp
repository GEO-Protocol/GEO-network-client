#include "CyclesBaseFiveOrSixNodesBoundaryMessage.h"

CyclesBaseFiveOrSixNodesBoundaryMessage::CyclesBaseFiveOrSixNodesBoundaryMessage(
    const SerializedEquivalent equivalent,
    vector<NodeUUID>& path,
    vector<NodeUUID>& boundaryNodes) :

    CycleBaseFiveOrSixNodesInBetweenMessage(
        equivalent,
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
    for (SerializedRecordNumber i=1; i<=boundaryNodesCount; i++){
        NodeUUID stepNodeUUID(buffer.get() + bytesBufferOffset);
        bytesBufferOffset += NodeUUID::kBytesSize;
        mBoundaryNodes.push_back(stepNodeUUID);
    }
}

pair<BytesShared, size_t> CyclesBaseFiveOrSixNodesBoundaryMessage::serializeToBytes() const
    throw(bad_alloc)
{
    auto parentBytesAndCount = CycleBaseFiveOrSixNodesInBetweenMessage::serializeToBytes();

    SerializedRecordsCount boundaryNodesCount = (SerializedRecordsCount) mBoundaryNodes.size();
    size_t bytesCount =
        parentBytesAndCount.second +
        (NodeUUID::kBytesSize) * boundaryNodesCount +
        sizeof(SerializedRecordsCount);
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
    vector<byte> stepObligationFlow;
    for(const auto &kNodeUUID: mBoundaryNodes){
        memcpy(
            dataBytesShared.get() + dataBytesOffset,
            &kNodeUUID,
            NodeUUID::kBytesSize);
        dataBytesOffset += NodeUUID::kBytesSize;
    }
    return make_pair(
        dataBytesShared,
        bytesCount);
}

const vector<NodeUUID> CyclesBaseFiveOrSixNodesBoundaryMessage::BoundaryNodes() const
{
    return mBoundaryNodes;
}