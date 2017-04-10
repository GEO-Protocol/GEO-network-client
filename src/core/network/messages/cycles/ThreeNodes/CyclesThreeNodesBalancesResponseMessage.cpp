#include "CyclesThreeNodesBalancesResponseMessage.h"

CyclesThreeNodesBalancesResponseMessage::CyclesThreeNodesBalancesResponseMessage(
    const NodeUUID &senderUUID,
    const TransactionUUID &transactionUUID,
    vector<NodeUUID> &neighbors) :
    TransactionMessage(senderUUID, transactionUUID),
    mNeighborsUUUID(neighbors)
{

}

CyclesThreeNodesBalancesResponseMessage::CyclesThreeNodesBalancesResponseMessage(
    const NodeUUID &senderUUID,
    const TransactionUUID &transactionUUID,
    uint16_t neighborsUUUIDCount):
    TransactionMessage(
            senderUUID,
            transactionUUID)
{
    mNeighborsUUUID.reserve(neighborsUUUIDCount);
}

std::pair<BytesShared, size_t> CyclesThreeNodesBalancesResponseMessage::serializeToBytes() {
    auto parentBytesAndCount = TransactionMessage::serializeToBytes();

    uint16_t boundaryNodesCount = (uint16_t) mNeighborsUUUID.size();
    size_t bytesCount =
            parentBytesAndCount.second +
            (NodeUUID::kBytesSize) * boundaryNodesCount +
            sizeof(boundaryNodesCount);
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
//    for mNeighborsBalances
    memcpy(
            dataBytesShared.get() + dataBytesOffset,
            &boundaryNodesCount,
            sizeof(uint16_t)
    );
    dataBytesOffset += sizeof(uint16_t);
    for(const auto &kNodeUUUID: mNeighborsUUUID){
        memcpy(
                dataBytesShared.get() + dataBytesOffset,
                &kNodeUUUID,
                NodeUUID::kBytesSize
        );
        dataBytesOffset += NodeUUID::kBytesSize;
    }
    return make_pair(
            dataBytesShared,
            bytesCount
    );
}

const Message::MessageType CyclesThreeNodesBalancesResponseMessage::typeID() const {
    return Message::MessageTypeID::Cycles_ThreeNodesBalancesReceiverMessage;
}

CyclesThreeNodesBalancesResponseMessage::CyclesThreeNodesBalancesResponseMessage(BytesShared buffer) {
    deserializeFromBytes(buffer);
}

void CyclesThreeNodesBalancesResponseMessage::deserializeFromBytes(BytesShared buffer) {
    TransactionMessage::deserializeFromBytes(buffer);
    size_t bytesBufferOffset = TransactionMessage::kOffsetToInheritedBytes();
//    Get NodesCount
    uint16_t boundaryNodesCount;
    memcpy(
            &boundaryNodesCount,
            buffer.get() + bytesBufferOffset,
            sizeof(uint16_t)
    );
    bytesBufferOffset += sizeof(uint16_t);
//    Parse boundary nodes
    NodeUUID stepNodeUUID;
    for (uint16_t i=1; i<=boundaryNodesCount; i++){
        memcpy(
                stepNodeUUID.data,
                buffer.get() + bytesBufferOffset,
                NodeUUID::kBytesSize
        );
        bytesBufferOffset += NodeUUID::kBytesSize;
        mNeighborsUUUID.push_back(stepNodeUUID);
    };
}

vector<NodeUUID> CyclesThreeNodesBalancesResponseMessage::NeighborsAndBalances() {
    return mNeighborsUUUID;
}

const bool CyclesThreeNodesBalancesResponseMessage::isTransactionMessage() const {
    return true;
}

void CyclesThreeNodesBalancesResponseMessage::addNeighborUUIDAndBalance(
    NodeUUID neighborUUID) {
    mNeighborsUUUID.push_back(neighborUUID);
}

