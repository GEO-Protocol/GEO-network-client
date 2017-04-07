#include "CyclesFourNodesBalancesResponseMessage.h"

CyclesFourNodesBalancesResponseMessage::CyclesFourNodesBalancesResponseMessage(
    const NodeUUID &senderUUID,
    const TransactionUUID &transactionUUID,
    vector<NodeUUID> &neighborsUUID):
        TransactionMessage(senderUUID, transactionUUID),
        mNeighborsUUID(neighborsUUID)
{}

CyclesFourNodesBalancesResponseMessage::CyclesFourNodesBalancesResponseMessage(
    const NodeUUID &senderUUID,
    const TransactionUUID &transactionUUID,
    uint16_t neighborsUUUIDCount):
    TransactionMessage(senderUUID, transactionUUID)
{
    mNeighborsUUID.reserve(neighborsUUUIDCount);
}

std::pair<BytesShared, size_t> CyclesFourNodesBalancesResponseMessage::serializeToBytes() {
    auto parentBytesAndCount = TransactionMessage::serializeToBytes();

    uint16_t neighborsNodesCount = (uint16_t) mNeighborsUUID.size();
    size_t bytesCount =
            parentBytesAndCount.second +
            (NodeUUID::kBytesSize) * neighborsNodesCount +
            sizeof(neighborsNodesCount);

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
    vector<byte> stepObligationFlow;
    // for mNeighborsUUIDs
    memcpy(
            dataBytesShared.get() + dataBytesOffset,
            &neighborsNodesCount,
            sizeof(uint16_t)
    );
    dataBytesOffset += sizeof(uint16_t);
    for(const auto &kNodeUUID: mNeighborsUUID){
        memcpy(
                dataBytesShared.get() + dataBytesOffset,
                &kNodeUUID,
                NodeUUID::kBytesSize
        );
        dataBytesOffset += NodeUUID::kBytesSize;
    }
    return make_pair(
            dataBytesShared,
            bytesCount
    );
}

const Message::MessageType CyclesFourNodesBalancesResponseMessage::typeID() const {
    return Message::MessageTypeID::Cycles_FourNodesBalancesResponseMessage;
}

CyclesFourNodesBalancesResponseMessage::CyclesFourNodesBalancesResponseMessage(BytesShared buffer) {
    deserializeFromBytes(buffer);
}

void CyclesFourNodesBalancesResponseMessage::deserializeFromBytes(BytesShared buffer) {
    TransactionMessage::deserializeFromBytes(buffer);
    size_t bytesBufferOffset = TransactionMessage::kOffsetToInheritedBytes();

    NodeUUID stepNodeUUID;
    //  Get neighborsNodesCount
    uint16_t neighborsNodesCount;
    memcpy(
            &neighborsNodesCount,
            buffer.get() + bytesBufferOffset,
            sizeof(uint16_t)
    );
    bytesBufferOffset += sizeof(uint16_t);
//    Parse mNeighborsUUIDS
    for (uint16_t i=1; i<=neighborsNodesCount; i++){
        memcpy(
                stepNodeUUID.data,
                buffer.get() + bytesBufferOffset,
                NodeUUID::kBytesSize
        );
        bytesBufferOffset += NodeUUID::kBytesSize;
        mNeighborsUUID.push_back(stepNodeUUID);
    };
}

const bool CyclesFourNodesBalancesResponseMessage::isTransactionMessage() const {
    return  true;
}

vector<NodeUUID> CyclesFourNodesBalancesResponseMessage::NeighborsUUID() {
    return mNeighborsUUID;
}

void CyclesFourNodesBalancesResponseMessage::AddNeighborUUID(
    NodeUUID neighborUUID) {
    mNeighborsUUID.push_back(neighborUUID);
}
