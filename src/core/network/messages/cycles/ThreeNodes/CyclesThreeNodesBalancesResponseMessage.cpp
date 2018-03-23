#include "CyclesThreeNodesBalancesResponseMessage.h"


CyclesThreeNodesBalancesResponseMessage::CyclesThreeNodesBalancesResponseMessage(
    const SerializedEquivalent equivalent,
    const NodeUUID &senderUUID,
    const TransactionUUID &transactionUUID,
    vector<NodeUUID> &neighbors) :
    TransactionMessage(
        equivalent,
        senderUUID,
        transactionUUID),
    mNeighborsUUUID(neighbors)
{}

CyclesThreeNodesBalancesResponseMessage::CyclesThreeNodesBalancesResponseMessage(
    BytesShared buffer):

    TransactionMessage(buffer)
{
    size_t bytesBufferOffset = TransactionMessage::kOffsetToInheritedBytes();
    //    Get NodesCount
    SerializedRecordsCount boundaryNodesCount;
    memcpy(
        &boundaryNodesCount,
        buffer.get() + bytesBufferOffset,
        sizeof(SerializedRecordsCount));
    bytesBufferOffset += sizeof(SerializedRecordsCount);
    //    Parse boundary nodes
    for (SerializedRecordNumber i=1; i<=boundaryNodesCount; i++){
        NodeUUID stepNodeUUID(buffer.get() + bytesBufferOffset);
        bytesBufferOffset += NodeUUID::kBytesSize;
        mNeighborsUUUID.push_back(stepNodeUUID);
    }
}

std::pair<BytesShared, size_t> CyclesThreeNodesBalancesResponseMessage::serializeToBytes() const
    throw (bad_alloc)
{
    auto parentBytesAndCount = TransactionMessage::serializeToBytes();

    auto boundaryNodesCount = (SerializedRecordsCount) mNeighborsUUUID.size();
    size_t bytesCount =
        parentBytesAndCount.second +
        (NodeUUID::kBytesSize) * boundaryNodesCount +
        sizeof(SerializedRecordsCount);
    BytesShared dataBytesShared = tryCalloc(bytesCount);
    size_t dataBytesOffset = 0;
    // for parent node
    //----------------------------------------------------
    memcpy(
        dataBytesShared.get(),
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second);
    dataBytesOffset += parentBytesAndCount.second;
    //    for mNeighborsBalances
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        &boundaryNodesCount,
        sizeof(SerializedRecordsCount));
    dataBytesOffset += sizeof(SerializedRecordsCount);
    for(const auto &kNodeUUUID: mNeighborsUUUID){
        memcpy(
            dataBytesShared.get() + dataBytesOffset,
            &kNodeUUUID,
            NodeUUID::kBytesSize);
        dataBytesOffset += NodeUUID::kBytesSize;
    }
    return make_pair(
        dataBytesShared,
        bytesCount);
}

const Message::MessageType CyclesThreeNodesBalancesResponseMessage::typeID() const
    noexcept
{
    return Message::MessageType::Cycles_ThreeNodesBalancesResponse;
}

vector<NodeUUID> CyclesThreeNodesBalancesResponseMessage::commonNodes()
    noexcept
{
    return mNeighborsUUUID;
}
