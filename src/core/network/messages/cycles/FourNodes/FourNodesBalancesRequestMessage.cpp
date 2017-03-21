#include "FourNodesBalancesRequestMessage.h"

pair<BytesShared, size_t> FourNodesBalancesRequestMessage::serializeToBytes() {
    vector<byte> MaxFlowBuffer = trustLineBalanceToBytes(mMaxFlow);
    auto parentBytesAndCount = TransactionMessage::serializeToBytes();
    uint16_t neighborsCreditorsCount = mNeighborsCreditor.size();
    uint16_t neighborsDebtorsCount = mNeighborsDebtor.size();
    size_t bytesCount = parentBytesAndCount.second
                        + sizeof(neighborsCreditorsCount)
                        + sizeof(neighborsDebtorsCount)
                        + neighborsCreditorsCount * NodeUUID::kBytesSize
                        + neighborsDebtorsCount * NodeUUID::kBytesSize
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
    // For mNeighborsCreditor
    memcpy(
            dataBytesShared.get() + dataBytesOffset,
            &neighborsCreditorsCount,
            sizeof(neighborsCreditorsCount)
    );
    dataBytesOffset += neighborsCreditorsCount;

    for(auto const& value: mNeighborsCreditor) {
        memcpy(
                dataBytesShared.get() + dataBytesOffset,
                &value,
                NodeUUID::kBytesSize
        );
        dataBytesOffset += NodeUUID::kBytesSize;
    }
    //----------------------------------------------------
//    For mNeighborsDebtor
    memcpy(
            dataBytesShared.get() + dataBytesOffset,
            &neighborsDebtorsCount,
            sizeof(neighborsDebtorsCount)
    );
    dataBytesOffset += neighborsDebtorsCount;

    for(auto const& value: mNeighborsDebtor) {
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
    uint16_t neighborsCreditorsCount;
    memcpy(
            &neighborsCreditorsCount,
            buffer.get() + bytesBufferOffset,
            sizeof(uint8_t)
    );
    bytesBufferOffset += sizeof(neighborsCreditorsCount);

    for (uint8_t i = 1; i <= neighborsCreditorsCount; ++i) {
        NodeUUID stepNode;
        memcpy(
                stepNode.data,
                buffer.get() + bytesBufferOffset,
                NodeUUID::kBytesSize
        );
        bytesBufferOffset += NodeUUID::kBytesSize;
        mNeighborsCreditor.push_back(stepNode);
    }
//    For mNeighborsDebtors
    uint16_t neighborsDebtorsCount;
    memcpy(
            &neighborsDebtorsCount,
            buffer.get() + bytesBufferOffset,
            sizeof(uint8_t)
    );
    bytesBufferOffset += sizeof(neighborsDebtorsCount);

    for (uint8_t i = 1; i <= neighborsDebtorsCount; ++i) {
        NodeUUID stepNode;
        memcpy(
                stepNode.data,
                buffer.get() + bytesBufferOffset,
                NodeUUID::kBytesSize
        );
        bytesBufferOffset += NodeUUID::kBytesSize;
        mNeighborsDebtor.push_back(stepNode);
    }
}

const Message::MessageType FourNodesBalancesRequestMessage::typeID() const {
    return Message::MessageTypeID::FourNodesBalancesRequestMessage;
}

FourNodesBalancesRequestMessage::FourNodesBalancesRequestMessage(BytesShared buffer) {
    deserializeFromBytes(buffer);
}

FourNodesBalancesRequestMessage::FourNodesBalancesRequestMessage(const TrustLineBalance &maxFlow,
                                                                 vector<NodeUUID> &neighborsDebtor,
                                                                 vector<NodeUUID> &neighborsCreditor):
    mMaxFlow(maxFlow),
    mNeighborsDebtor(neighborsDebtor),
    mNeighborsCreditor(neighborsCreditor)
{

}

vector<NodeUUID> FourNodesBalancesRequestMessage::NeighborsDebtor() {
    return mNeighborsDebtor;
}

vector<NodeUUID> FourNodesBalancesRequestMessage::NeighborsCreditor() {
    return mNeighborsCreditor;
}
