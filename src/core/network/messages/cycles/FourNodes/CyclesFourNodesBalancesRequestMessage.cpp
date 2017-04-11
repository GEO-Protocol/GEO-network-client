#include "CyclesFourNodesBalancesRequestMessage.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wconversion"
pair<BytesShared, size_t> CyclesFourNodesBalancesRequestMessage::serializeToBytes() {
    auto parentBytesAndCount = TransactionMessage::serializeToBytes();
    const uint16_t neighborsCount = mNeighbors.size();
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
    dataBytesOffset += sizeof(neighborsCount);

    for(auto const& kNodeUUID: mNeighbors) {
        memcpy(
                dataBytesShared.get() + dataBytesOffset,
                &kNodeUUID,
                NodeUUID::kBytesSize
        );
        dataBytesOffset += NodeUUID::kBytesSize;
    }
    //----------------------------------------------------
//    cout << "_______________________________" << endl;
//    cout << "CyclesFourNodesBalancesRequestMessage::serializeToBytes" << endl;
//    cout << "Transaction UUID: " << this->transactionUUID() << endl;
//    cout << "_______________________________" << endl;
    return make_pair(
            dataBytesShared,
            bytesCount
    );
}
#pragma clang diagnostic pop

void CyclesFourNodesBalancesRequestMessage::deserializeFromBytes(
        BytesShared buffer) {

    TransactionMessage::deserializeFromBytes(buffer);
    size_t bytesBufferOffset = TransactionMessage::kOffsetToInheritedBytes();
    // path
    uint16_t neighborsCount;
    memcpy(
            &neighborsCount,
            buffer.get() + bytesBufferOffset,
            sizeof(neighborsCount)
    );
    bytesBufferOffset += sizeof(neighborsCount);

    for (uint16_t i = 1; i <= neighborsCount; ++i) {
        NodeUUID stepNode;
        memcpy(
                stepNode.data,
                buffer.get() + bytesBufferOffset,
                NodeUUID::kBytesSize
        );
        bytesBufferOffset += NodeUUID::kBytesSize;
        mNeighbors.insert(stepNode);
    }
//    cout << "_______________________________" << endl;
//    cout << "CyclesFourNodesBalancesRequestMessage::deserializeFromByte" << endl;
//    cout << "Transaction UUID: " << this->transactionUUID() << endl;
//    cout << "_______________________________" << endl;
}

const Message::MessageType CyclesFourNodesBalancesRequestMessage::typeID() const {
    return Message::MessageTypeID::Cycles_FourNodesBalancesRequestMessage;
}

CyclesFourNodesBalancesRequestMessage::CyclesFourNodesBalancesRequestMessage(BytesShared buffer) {
    deserializeFromBytes(buffer);
}

CyclesFourNodesBalancesRequestMessage::CyclesFourNodesBalancesRequestMessage(
    const NodeUUID &senderUUID,
    const TransactionUUID &transactionUUID,
    set<NodeUUID> &neighbors):
    TransactionMessage(senderUUID, transactionUUID),
    mNeighbors(neighbors)
{

}

set<NodeUUID> CyclesFourNodesBalancesRequestMessage::Neighbors() {
    return mNeighbors;
}