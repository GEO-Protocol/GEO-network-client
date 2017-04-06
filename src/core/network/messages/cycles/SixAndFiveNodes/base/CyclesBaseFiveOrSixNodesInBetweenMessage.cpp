#include "CyclesBaseFiveOrSixNodesInBetweenMessage.h"

CycleBaseFiveOrSixNodesInBetweenMessage::CycleBaseFiveOrSixNodesInBetweenMessage() {}

CycleBaseFiveOrSixNodesInBetweenMessage::CycleBaseFiveOrSixNodesInBetweenMessage(
    vector<NodeUUID> &path) :
    mPath(path)
{
    mNodesInPath = (uint8_t) mPath.size();
}

CycleBaseFiveOrSixNodesInBetweenMessage::CycleBaseFiveOrSixNodesInBetweenMessage(
    BytesShared buffer) {

    deserializeFromBytes(buffer);
}

pair<BytesShared, size_t> CycleBaseFiveOrSixNodesInBetweenMessage::serializeToBytes() {
    size_t bytesCount = sizeof(MessageType)
                        + sizeof(mNodesInPath)
                        + mNodesInPath * NodeUUID::kBytesSize;
    BytesShared dataBytesShared = tryCalloc(bytesCount);
    size_t dataBytesOffset = 0;

//    MessageType
    MessageType messageType = (MessageType) typeID();
    memcpy(
        dataBytesShared.get(),
        &messageType,
        sizeof(MessageType)
    );
    dataBytesOffset += sizeof(MessageType);
    // For path
    // Write vector size first
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        &mNodesInPath,
        sizeof(mNodesInPath)
    );
    dataBytesOffset += sizeof(mNodesInPath);

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
    memcpy(
        &mNodesInPath,
        buffer.get() + bytesBufferOffset,
        sizeof(uint8_t)
    );
    bytesBufferOffset += sizeof(uint8_t);

    for (uint8_t i = 1; i <= mNodesInPath; ++i) {
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
    static const size_t offset =
        //            node in path
        + sizeof(uint8_t)
        + NodeUUID::kBytesSize * mNodesInPath
        + sizeof(MessageType);
    return offset;
}


const vector<NodeUUID> CycleBaseFiveOrSixNodesInBetweenMessage::Path() const {
    return mPath;
}

void CycleBaseFiveOrSixNodesInBetweenMessage::addNodeToPath(NodeUUID InBetweenNode) {
    mPath.push_back(InBetweenNode);
}
