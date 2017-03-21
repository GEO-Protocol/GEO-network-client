#include "ThreeNodesBalancesRequestMessage.h"

pair<BytesShared, size_t> ThreeNodesBalancesRequestMessage::serializeToBytes() {
    vector<byte> MaxFlowBuffer = trustLineBalanceToBytes(mMaxFlow);
    auto parentBytesAndCount = TransactionMessage::serializeToBytes();
    uint16_t neighborsCount = mNeighbors.size();
    size_t bytesCount = parentBytesAndCount.second
                        + neighborsCount
                        + mNeighbors.size() * NodeUUID::kBytesSize
                        + MaxFlowBuffer.size();

    BytesShared dataBytesShared = tryCalloc(bytesCount);
    size_t dataBytesOffset = 0;
    //----------------------------------------------------
    memcpy(
            dataBytesShared.get(),
            parentBytesAndCount.first.get(),
            parentBytesAndCount.second
    );
    dataBytesOffset += parentBytesAndCount.second;
    // for max flow
    memcpy(
            dataBytesShared.get() + dataBytesOffset,
            MaxFlowBuffer.data(),
            MaxFlowBuffer.size()
    );
    dataBytesOffset += MaxFlowBuffer.size();
    // For path
    memcpy(
            dataBytesShared.get() + dataBytesOffset,
            &neighborsCount,
            sizeof(neighborsCount)
    );
    dataBytesOffset += neighborsCount;

    for(auto const& value: mNeighbors) {
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

void ThreeNodesBalancesRequestMessage::deserializeFromBytes(
        BytesShared buffer) {

    SenderMessage::deserializeFromBytes(buffer);
    size_t bytesBufferOffset = TransactionMessage::kOffsetToInheritedBytes();

    uint16_t neighborsCount;
    //   // Max flow
    vector<byte> amountBytes(
            buffer.get() + bytesBufferOffset,
            buffer.get() + bytesBufferOffset + kTrustLineBalanceSerializeBytesCount);


    mMaxFlow = bytesToTrustLineBalance(amountBytes);
    bytesBufferOffset += kTrustLineBalanceSerializeBytesCount;
    // path
    memcpy(
            &neighborsCount,
            buffer.get() + bytesBufferOffset,
            sizeof(uint8_t)
    );
    bytesBufferOffset += sizeof(neighborsCount);

    for (uint8_t i = 1; i <= neighborsCount; ++i) {
        NodeUUID stepNode;
        memcpy(
                stepNode.data,
                buffer.get() + bytesBufferOffset,
                NodeUUID::kBytesSize
        );
        bytesBufferOffset += NodeUUID::kBytesSize;
        mNeighbors.push_back(stepNode);
    }
}

const Message::MessageType ThreeNodesBalancesRequestMessage::typeID() const {
    return Message::MessageTypeID::ThreeNodesBalancesRequestMessage;
}

ThreeNodesBalancesRequestMessage::ThreeNodesBalancesRequestMessage() {

}

ThreeNodesBalancesRequestMessage::ThreeNodesBalancesRequestMessage(BytesShared buffer) {
    deserializeFromBytes(buffer);
}

const size_t ThreeNodesBalancesRequestMessage::kOffsetToInheritedBytes() {
    return 0;
}

ThreeNodesBalancesRequestMessage::ThreeNodesBalancesRequestMessage(const TrustLineBalance &maxFlow, vector<NodeUUID> &neighbors):
        mNeighbors(neighbors),
        mMaxFlow(maxFlow)
{

}

vector<NodeUUID> ThreeNodesBalancesRequestMessage::Neighbors() {
    return mNeighbors;
}
