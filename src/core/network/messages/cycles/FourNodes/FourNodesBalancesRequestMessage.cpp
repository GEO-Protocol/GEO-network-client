#include "FourNodesBalancesRequestMessage.h"

pair<BytesShared, size_t> FourNodesBalancesRequestMessage::serializeToBytes() {
    vector<byte> MaxFlowBuffer = trustLineBalanceToBytes(mMaxFlow);
    auto parentBytesAndCount = TransactionMessage::serializeToBytes();
    uint16_t neighborsCount = mNeighbors.size();
    size_t bytesCount = parentBytesAndCount.second
                        + sizeof(neighborsCount)
                        + neighborsCount * NodeUUID::kBytesSize
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
    //----------------------------------------------------
//    For mNeighbors
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

void FourNodesBalancesRequestMessage::deserializeFromBytes(
        BytesShared buffer) {

    TransactionMessage::deserializeFromBytes(buffer);
    size_t bytesBufferOffset = TransactionMessage::kOffsetToInheritedBytes();
    //   // Max flow
    vector<byte> amountBytes(
            buffer.get() + bytesBufferOffset,
            buffer.get() + bytesBufferOffset + kTrustLineBalanceSerializeBytesCount);


    mMaxFlow = bytesToTrustLineBalance(amountBytes);
    bytesBufferOffset += kTrustLineBalanceSerializeBytesCount;
    // path
    uint16_t neighborsCount;
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

const Message::MessageType FourNodesBalancesRequestMessage::typeID() const {
    return Message::MessageTypeID::FourNodesBalancesRequestMessage;
}

FourNodesBalancesRequestMessage::FourNodesBalancesRequestMessage(BytesShared buffer) {
    deserializeFromBytes(buffer);
}

FourNodesBalancesRequestMessage::FourNodesBalancesRequestMessage(const TrustLineBalance &maxFlow,
                                                                 vector<NodeUUID> &neighbors):
    mMaxFlow(maxFlow),
    mNeighbors(neighbors)
{

}

vector<NodeUUID> FourNodesBalancesRequestMessage::Neighbors() {
    return mNeighbors;
}

TrustLineBalance FourNodesBalancesRequestMessage::MaxFlow() {
    return mMaxFlow;
}
