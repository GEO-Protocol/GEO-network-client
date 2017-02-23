#include "InBetweenNodeTopologyMessage.h"

uint8_t InBetweenNodeTopologyMessage::mNodesInPath;

InBetweenNodeTopologyMessage::InBetweenNodeTopologyMessage(
        const TrustLineBalance maxFlow, //TODO:: (D.V.) TrustLineBalance is non primitive type, use address, don't copy tmp value in constructor anymore.
        const byte max_depth,
        vector<NodeUUID> &path) : //TODO:: (D.V.) Try to use move semantic.

    mMaxFlow(maxFlow) { //TODO:: (D.V.) You can do like this.

    mMaxDepth = max_depth;
    mPath = path;
    mNodesInPath = (uint8_t) mPath.size(); //TODO:: Are you really sure that this value always will be in range of uint_8 type?
}

InBetweenNodeTopologyMessage::InBetweenNodeTopologyMessage(
    BytesShared buffer) {

    deserializeFromBytes(buffer);
}

const Message::MessageType InBetweenNodeTopologyMessage::typeID() const {

    return Message::InBetweenNodeTopologyMessage;
}

//TODO:: (D.V.) In most cases, "getter" returns const value, and address, not copy.
//TODO:: Also, getter must be declared as a const function. It's deny to modify inner object state.
TrustLineBalance InBetweenNodeTopologyMessage::getMaxFlow() {

    return mMaxFlow;
}

//TODO:: (D.V.) In most cases, "getter" returns const value, and address, not copy.
//TODO:: Also, getter must be declared as a const function. It's deny to modify inner object state.
vector<NodeUUID> InBetweenNodeTopologyMessage::getPath() {

    //TODO:: (D.V.) Empty vector?
    return vector<NodeUUID>();
}

pair<BytesShared, size_t> InBetweenNodeTopologyMessage::serializeToBytes() {

    //TODO:: (D.V.) How can you call Message's method serializeToBytes() if you don't set him field-members ??!!
    //TODO:: But now this is doesn't matter. Messages hierarchy has changed. I will comment this part of code.

    //auto parentBytesAndCount = Message::serializeToBytes();

    vector<byte> MaxFlowBuffer = trustLineBalanceToBytes(mMaxFlow);

    size_t bytesCount = MaxFlowBuffer.size()
                        + sizeof(mMaxDepth)
                        + sizeof(mNodesInPath)
                        + mNodesInPath * NodeUUID::kBytesSize;

    BytesShared dataBytesShared = tryCalloc(bytesCount);
    size_t dataBytesOffset = 0;

    // for parent node
    //----------------------------------------------------
    /*memcpy(
            dataBytesShared.get(),
            parentBytesAndCount.first.get(),
            parentBytesAndCount.second
    );
    dataBytesOffset += parentBytesAndCount.second;*/
    //----------------------------------------------------
    // for max flow
    memcpy(
        dataBytesShared.get(),
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
        //dataBytesOffset += NodeUUID::kBytesSize;
    }
    //----------------------------------------------------
    return make_pair(
            dataBytesShared,
            bytesCount
    );
}

void InBetweenNodeTopologyMessage::deserializeFromBytes(
        BytesShared buffer) {

    //TODO:: (D.V.) Same problem when you invoke serializeToBytes().
    //Message::deserializeFromBytes(buffer);
    // Parent part of deserializeFromBytes
    //size_t bytesBufferOffset = Message::kOffsetToInheritedBytes();

    vector<byte> amountBytes(
            buffer.get(),
            buffer.get() + kTrustLineBalanceSerializeBytesCount);

    // Max flow
    mMaxFlow = bytesToTrustLineBalance(amountBytes);
    size_t bytesBufferOffset = kTrustLineBalanceSerializeBytesCount;

    // for max depth
    memcpy(
            &mMaxDepth,
            buffer.get() + bytesBufferOffset,
            sizeof(uint8_t)
    );
    bytesBufferOffset += sizeof(uint8_t);

    // path
    uint8_t nodes_in_path;
    memcpy(
            &nodes_in_path,
            buffer.get() + bytesBufferOffset,
            sizeof(uint8_t)
    );
    bytesBufferOffset += sizeof(uint8_t);

    NodeUUID stepNode;
    for (uint8_t i = 1; i <= nodes_in_path; ++i) {
        memcpy(
            stepNode.data,
            buffer.get() + bytesBufferOffset,
            NodeUUID::kBytesSize
        );
        bytesBufferOffset += NodeUUID::kBytesSize;
        mPath.push_back(stepNode);
    }
}

const size_t InBetweenNodeTopologyMessage::kOffsetToInheritedBytes() {

// todo add sizeof message
// todo Ask about static for this object
    static const size_t offset =
            + kTrustLineBalanceSerializeBytesCount
            + sizeof(mMaxDepth)
            + sizeof(uint8_t)
            + NodeUUID::kBytesSize * mNodesInPath;

    return offset;
}