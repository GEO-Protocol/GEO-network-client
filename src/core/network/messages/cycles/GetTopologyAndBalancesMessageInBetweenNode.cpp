#include "GetTopologyAndBalancesMessageInBetweenNode.h"

GetTopologyAndBalancesMessageInBetweenNode::GetTopologyAndBalancesMessageInBetweenNode(const TrustLineAmount maxFlow,
                                                                                         const byte max_depth,
                                                                                         vector<NodeUUID> &path) {
    mMaxFlow = maxFlow;
    mMax_depth = max_depth;
    mPath = path;
    mNodesInPath = (uint8_t) mPath.size();
}

GetTopologyAndBalancesMessageInBetweenNode::GetTopologyAndBalancesMessageInBetweenNode(BytesShared buffer) {
    deserializeFromBytes(buffer);
}

pair<BytesShared, size_t> GetTopologyAndBalancesMessageInBetweenNode::serializeToBytes() {
    auto parentBytesAndCount = TransactionMessage::serializeToBytes();


    vector<byte> MaxFlowBuffer = trustLineAmountToBytes(mMaxFlow);

    size_t bytesCount = parentBytesAndCount.second +
                                MaxFlowBuffer.size() +
                                sizeof(mMax_depth) +
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
            &mMax_depth,
            sizeof(mMax_depth)
    );
    dataBytesOffset += sizeof(mMax_depth);
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

void GetTopologyAndBalancesMessageInBetweenNode::deserializeFromBytes(
        BytesShared buffer) {
    TransactionMessage::deserializeFromBytes(buffer);
//    Parent part of deserializeFromBytes
    size_t bytesBufferOffset = TransactionMessage::kOffsetToInheritedBytes();
    vector<byte> amountBytes(
            buffer.get() + bytesBufferOffset,
            buffer.get() + bytesBufferOffset + kTrustLineAmountBytesCount);
//    Max flow
    mMaxFlow = bytesToTrustLineAmount(amountBytes);
    bytesBufferOffset += kTrustLineAmountBytesCount;
// for max depth
    memcpy(
            &mMax_depth,
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

const Message::MessageType GetTopologyAndBalancesMessageInBetweenNode::typeID() const {
    return Message::GetTopologyAndBalancesMessageFirstLevelNode;
}

const TransactionUUID &GetTopologyAndBalancesMessageInBetweenNode::transactionUUID() const {
    throw NotImplementedError("OpenTrustLineMessage::deserializeFromBytes: "
                                      "Method not implemented.");
}

const TrustLineUUID &GetTopologyAndBalancesMessageInBetweenNode::trustLineUUID() const {
    throw NotImplementedError("OpenTrustLineMessage::deserializeFromBytes: "
                                      "Method not implemented.");
}

const size_t GetTopologyAndBalancesMessageInBetweenNode::kOffsetToInheritedBytes() {
//    todo add sizeof message
//    todo Ask about static for this object
    static const size_t offset =
            TransactionMessage::kOffsetToInheritedBytes()
            + kTrustLineAmountBytesCount
            + sizeof(mMax_depth)
            + sizeof(uint8_t)
            + NodeUUID::kBytesSize * mNodesInPath;
    ;
    return offset;
}
