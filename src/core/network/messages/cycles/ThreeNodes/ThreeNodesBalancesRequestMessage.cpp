#include "ThreeNodesBalancesRequestMessage.h"

ThreeNodesBalancesRequestMessage::ThreeNodesBalancesRequestMessage(BytesShared buffer)
{
    deserializeFromBytes(buffer);
}


ThreeNodesBalancesRequestMessage::ThreeNodesBalancesRequestMessage(
        const NodeUUID &senderUUID,
        const TransactionUUID &transactionUUID,
        set<NodeUUID> &neighbors):
        TransactionMessage(
                senderUUID,
                transactionUUID)
{
    mNeighbors.reserve(neighbors.size());
    for(auto &value: neighbors)
        mNeighbors.push_back(value);
}

pair<BytesShared, size_t> ThreeNodesBalancesRequestMessage::serializeToBytes() {
    auto parentBytesAndCount = TransactionMessage::serializeToBytes();
    uint16_t neighborsCount = mNeighbors.size();
    size_t bytesCount = parentBytesAndCount.second
                        + neighborsCount
                        + mNeighbors.size() * NodeUUID::kBytesSize;

    BytesShared dataBytesShared = tryCalloc(bytesCount);
    size_t dataBytesOffset = 0;
    //----------------------------------------------------
    memcpy(
            dataBytesShared.get(),
            parentBytesAndCount.first.get(),
            parentBytesAndCount.second
    );
    dataBytesOffset += parentBytesAndCount.second;
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
    return Message::MessageTypeID::Cycles_ThreeNodesBalancesRequestMessage;
}

vector<NodeUUID> ThreeNodesBalancesRequestMessage::Neighbors() {
    return mNeighbors;
}
