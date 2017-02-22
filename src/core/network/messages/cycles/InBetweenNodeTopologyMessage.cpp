#include "InBetweenNodeTopologyMessage.h"

uint8_t InBetweenNodeTopologyMessage::mNodesInPath;

InBetweenNodeTopologyMessage::InBetweenNodeTopologyMessage(
        const TrustLineBalance maxFlow,
        const byte max_depth,
        vector<NodeUUID> &path
       ){
    mMaxFlow = maxFlow;
    mMaxDepth = max_depth;
    mPath = path;
    mNodesInPath = (uint8_t) mPath.size();
}

InBetweenNodeTopologyMessage::InBetweenNodeTopologyMessage(BytesShared buffer) {
    deserializeFromBytes(buffer);
}

pair<BytesShared, size_t> InBetweenNodeTopologyMessage::serializeToBytes() {
    auto parentBytesAndCount = Message::serializeToBytes();


    vector<byte> MaxFlowBuffer = trustLineBalanceToBytes(mMaxFlow);

    size_t bytesCount = parentBytesAndCount.second +
                                MaxFlowBuffer.size() +
                                sizeof(mMaxDepth) +
                                sizeof(mNodesInPath) +
                                mNodesInPath * NodeUUID::kBytesSize;

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
    //----------------------------------------------------
    // for max flow
    memcpy(
            dataBytesShared.get() + dataBytesOffset,
            MaxFlowBuffer.data(),
            MaxFlowBuffer.size()
    );
    dataBytesOffset += MaxFlowBuffer.size();
    //----------------------------------------------------
    // for max depth
    memcpy(
            dataBytesShared.get() + dataBytesOffset,
            &mMaxDepth,
            sizeof(mMaxDepth)
    );
    dataBytesOffset += sizeof(mMaxDepth);
    //----------------------------------------------------
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
//        dataBytesOffset += NodeUUID::kBytesSize;
    }
    return make_pair(
            dataBytesShared,
            bytesCount
    );
}

void InBetweenNodeTopologyMessage::deserializeFromBytes(
        BytesShared buffer) {
    Message::deserializeFromBytes(buffer);
//    Parent part of deserializeFromBytes
    size_t bytesBufferOffset = Message::kOffsetToInheritedBytes();
    vector<byte> amountBytes(
            buffer.get() + bytesBufferOffset,
            buffer.get() + bytesBufferOffset + kTrustLineBalanceSerializeBytesCount);
//    Max flow
    mMaxFlow = bytesToTrustLineBalance(amountBytes);
    bytesBufferOffset += kTrustLineBalanceSerializeBytesCount;
// for max depth
    memcpy(
            &mMaxDepth,
            buffer.get() + bytesBufferOffset,
            sizeof(uint8_t)
    );
    bytesBufferOffset += sizeof(uint8_t);
//
// path
    uint8_t nodes_in_path;
    memcpy(
            &nodes_in_path,
            buffer.get() + bytesBufferOffset,
            sizeof(uint8_t)
    );
    bytesBufferOffset += sizeof(uint8_t);
    NodeUUID stepNode;
    for (uint8_t i = 1; i <= nodes_in_path; ++i){
        memcpy(
            stepNode.data,
            buffer.get() + bytesBufferOffset,
            NodeUUID::kBytesSize
        );
        bytesBufferOffset += NodeUUID::kBytesSize;
        mPath.push_back(stepNode);
    }
}

const Message::MessageType InBetweenNodeTopologyMessage::typeID() const {
    return Message::InBetweenNodeTopologyMessage;
}

const TransactionUUID &InBetweenNodeTopologyMessage::transactionUUID() const {
    throw NotImplementedError("OpenTrustLineMessage::deserializeFromBytes: "
                                      "Method not implemented.");
}

const TrustLineUUID &InBetweenNodeTopologyMessage::trustLineUUID() const {
    throw NotImplementedError("OpenTrustLineMessage::deserializeFromBytes: "
                                      "Method not implemented.");
}

const size_t InBetweenNodeTopologyMessage::kOffsetToInheritedBytes() {
//    todo add sizeof message
//    todo Ask about static for this object
    static const size_t offset =
            Message::kOffsetToInheritedBytes()
            + kTrustLineBalanceSerializeBytesCount
            + sizeof(mMaxDepth)
            + sizeof(uint8_t)
            + NodeUUID::kBytesSize * mNodesInPath;
    ;
    return offset;
}

TrustLineBalance InBetweenNodeTopologyMessage::getMaxFlow() {
    return mMaxFlow;
}

vector<NodeUUID> InBetweenNodeTopologyMessage::getPath() {
    return vector<NodeUUID>();
}
