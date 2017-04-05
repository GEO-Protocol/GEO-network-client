#include "FourNodesBalancesRequestMessage.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wconversion"
pair<BytesShared, size_t> FourNodesBalancesRequestMessage::serializeToBytes() {
    auto parentBytesAndCount = TransactionMessage::serializeToBytes();
    uint16_t neighborsCount = mNeighbors.size();
    size_t bytesCount = parentBytesAndCount.second
                        + sizeof(neighborsCount)
                        + neighborsCount * NodeUUID::kBytesSize;

    BytesShared dataBytesShared = tryCalloc(bytesCount);
    size_t dataBytesOffset = 0;
    //----------------------------------------------------
    memcpy(
            dataBytesShared.get(),
            parentBytesAndCount.first.get(),
            parentBytesAndCount.second
    );
    dataBytesOffset += parentBytesAndCount.second;
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
#pragma clang diagnostic pop

void FourNodesBalancesRequestMessage::deserializeFromBytes(
        BytesShared buffer) {

    TransactionMessage::deserializeFromBytes(buffer);
    size_t bytesBufferOffset = TransactionMessage::kOffsetToInheritedBytes();
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
        mNeighbors.insert(stepNode);
    }
}

const Message::MessageType FourNodesBalancesRequestMessage::typeID() const {
    return Message::MessageTypeID::FourNodesBalancesRequestMessage;
}

FourNodesBalancesRequestMessage::FourNodesBalancesRequestMessage(BytesShared buffer) {
    deserializeFromBytes(buffer);
}

FourNodesBalancesRequestMessage::FourNodesBalancesRequestMessage(
    const NodeUUID &senderUUID,
    const TransactionUUID &transactionUUID,
    set<NodeUUID> &neighbors):
    TransactionMessage(senderUUID, transactionUUID),
    mNeighbors(neighbors)
{

}

set<NodeUUID> FourNodesBalancesRequestMessage::Neighbors() {
    return mNeighbors;
}