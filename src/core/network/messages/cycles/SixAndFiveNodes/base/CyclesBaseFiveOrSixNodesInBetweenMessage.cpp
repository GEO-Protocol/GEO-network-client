#include "CyclesBaseFiveOrSixNodesInBetweenMessage.h"

CycleBaseFiveOrSixNodesInBetweenMessage::CycleBaseFiveOrSixNodesInBetweenMessage() {}

CycleBaseFiveOrSixNodesInBetweenMessage::CycleBaseFiveOrSixNodesInBetweenMessage(
    vector<NodeUUID> &path):
    mPath(path)
{}

CycleBaseFiveOrSixNodesInBetweenMessage::CycleBaseFiveOrSixNodesInBetweenMessage(
    BytesShared buffer) {

    deserializeFromBytes(buffer);
}

pair<BytesShared, size_t> CycleBaseFiveOrSixNodesInBetweenMessage::serializeToBytes() const
    throw(bad_alloc)
{
    auto parentBytesAndCount = Message::serializeToBytes();
    const uint8_t kNodesInPath = mPath.size();
    size_t bytesCount = parentBytesAndCount.second
                        + sizeof(kNodesInPath)
                        + kNodesInPath * NodeUUID::kBytesSize;
    BytesShared dataBytesShared = tryCalloc(bytesCount);
    size_t dataBytesOffset = 0;
    // for parent node
    //----------------------------------------------------
    memcpy(
            dataBytesShared.get(),
            parentBytesAndCount.first.get(),
            parentBytesAndCount.second
    );
    dataBytesOffset += parentBytesAndCount.second;
    // For path
    // Write vector size first
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        &kNodesInPath,
        sizeof(kNodesInPath)
    );
    dataBytesOffset += sizeof(kNodesInPath);

    for(auto const& value: mPath) {
        memcpy(
            dataBytesShared.get() + dataBytesOffset,
            &value,
            NodeUUID::kBytesSize
        );
        dataBytesOffset += NodeUUID::kBytesSize;
    }
    //----------------------------------------------------
    return make_pair(
        dataBytesShared,
        bytesCount
    );
}

void CycleBaseFiveOrSixNodesInBetweenMessage::deserializeFromBytes(
    BytesShared buffer) {

// Message Type
    size_t bytesBufferOffset = 0;
    MessageType *messageType = new (buffer.get()) MessageType;
    bytesBufferOffset += sizeof(uint16_t);
    // path
    uint8_t nodesInPath;
    memcpy(
        &nodesInPath,
        buffer.get() + bytesBufferOffset,
        sizeof(uint8_t)
    );
    bytesBufferOffset += sizeof(uint8_t);
    if (nodesInPath <= 0)
        return;
    for (uint8_t i = 1; i <= nodesInPath; ++i) {
        NodeUUID stepNode;
        memcpy(
            stepNode.data,
            buffer.get() + bytesBufferOffset,
            NodeUUID::kBytesSize
        );
        bytesBufferOffset += NodeUUID::kBytesSize;
        mPath.push_back(stepNode);
    }
}

const size_t CycleBaseFiveOrSixNodesInBetweenMessage::kOffsetToInheritedBytes() {
    const uint8_t kNodesInPath = mPath.size();
    const size_t offset =
        + sizeof(kNodesInPath)
        + NodeUUID::kBytesSize * kNodesInPath
        + Message::kOffsetToInheritedBytes();
    return offset;
}


const vector<NodeUUID> CycleBaseFiveOrSixNodesInBetweenMessage::Path() const {
    return mPath;
}

void CycleBaseFiveOrSixNodesInBetweenMessage::addNodeToPath(NodeUUID InBetweenNode) {
    mPath.push_back(InBetweenNode);
}
