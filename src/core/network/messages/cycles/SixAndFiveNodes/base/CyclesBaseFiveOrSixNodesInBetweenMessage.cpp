#include "CyclesBaseFiveOrSixNodesInBetweenMessage.h"

CycleBaseFiveOrSixNodesInBetweenMessage::CycleBaseFiveOrSixNodesInBetweenMessage(
    const SerializedEquivalent equivalent,
    vector<NodeUUID> &path):
    EquivalentMessage(
        equivalent),
    mPath(path)
{}

CycleBaseFiveOrSixNodesInBetweenMessage::CycleBaseFiveOrSixNodesInBetweenMessage(
    BytesShared buffer):
    EquivalentMessage(buffer)
{
    size_t bytesBufferOffset = EquivalentMessage::kOffsetToInheritedBytes();

    // path
    SerializedPositionInPath nodesInPath;
    memcpy(
        &nodesInPath,
        buffer.get() + bytesBufferOffset,
        sizeof(SerializedPathLengthSize));
    bytesBufferOffset += sizeof(SerializedPathLengthSize);
    if (nodesInPath <= 0)
        return;
    for (SerializedPositionInPath i = 1; i <= nodesInPath; ++i) {
        NodeUUID stepNode(buffer.get() + bytesBufferOffset);
        bytesBufferOffset += NodeUUID::kBytesSize;
        mPath.push_back(stepNode);
    }
}

pair<BytesShared, size_t> CycleBaseFiveOrSixNodesInBetweenMessage::serializeToBytes() const
    throw(bad_alloc)
{
    auto parentBytesAndCount = EquivalentMessage::serializeToBytes();
    const SerializedPathLengthSize kNodesInPath = (SerializedPathLengthSize)mPath.size();
    size_t bytesCount = parentBytesAndCount.second
                        + sizeof(SerializedPathLengthSize)
                        + kNodesInPath * NodeUUID::kBytesSize;
    BytesShared dataBytesShared = tryCalloc(bytesCount);
    size_t dataBytesOffset = 0;
    // for parent node
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

    for(auto const& value: mPath) {
        memcpy(
            dataBytesShared.get() + dataBytesOffset,
            &value,
            NodeUUID::kBytesSize);
        dataBytesOffset += NodeUUID::kBytesSize;
    }
    //----------------------------------------------------
    return make_pair(
        dataBytesShared,
        bytesCount);
}

const size_t CycleBaseFiveOrSixNodesInBetweenMessage::kOffsetToInheritedBytes()
{
    const SerializedPathLengthSize kNodesInPath = (SerializedPathLengthSize)mPath.size();
    const size_t offset =
        + sizeof(kNodesInPath)
        + NodeUUID::kBytesSize * kNodesInPath
        + EquivalentMessage::kOffsetToInheritedBytes();
    return offset;
}


const vector<NodeUUID> CycleBaseFiveOrSixNodesInBetweenMessage::Path() const
{
    return mPath;
}

void CycleBaseFiveOrSixNodesInBetweenMessage::addNodeToPath(
    const NodeUUID &inBetweenNode)
{
    mPath.push_back(inBetweenNode);
}
