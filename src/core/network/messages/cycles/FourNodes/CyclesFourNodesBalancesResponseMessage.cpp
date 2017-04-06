#include "CyclesFourNodesBalancesResponseMessage.h"

CyclesFourNodesBalancesResponseMessage::CyclesFourNodesBalancesResponseMessage(
    const NodeUUID &senderUUID,
    const TransactionUUID &transactionUUID,
    vector<pair<NodeUUID, TrustLineBalance>> &neighborsBalances):
        TransactionMessage(senderUUID, transactionUUID),
        mNeighborsBalances(neighborsBalances)
{

}

CyclesFourNodesBalancesResponseMessage::CyclesFourNodesBalancesResponseMessage(
    const NodeUUID &senderUUID,
    const TransactionUUID &transactionUUID,
    uint16_t neighborsUUUIDAndBalancesCount):
    TransactionMessage(senderUUID, transactionUUID)
{
    mNeighborsBalances.reserve(neighborsUUUIDAndBalancesCount);
}

std::pair<BytesShared, size_t> CyclesFourNodesBalancesResponseMessage::serializeToBytes() {
    auto parentBytesAndCount = TransactionMessage::serializeToBytes();

    uint16_t neighborsNodesCount = (uint16_t) mNeighborsBalances.size();
    size_t bytesCount =
            parentBytesAndCount.second +
            (NodeUUID::kBytesSize + kTrustLineBalanceSerializeBytesCount) * neighborsNodesCount +
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
// for mNeighborsBalances
    memcpy(
            dataBytesShared.get() + dataBytesOffset,
            &neighborsNodesCount,
            sizeof(uint16_t)
    );
    dataBytesOffset += sizeof(uint16_t);
    for(auto &value: mNeighborsBalances){
        memcpy(
                dataBytesShared.get() + dataBytesOffset,
                &value.first,
                NodeUUID::kBytesSize
        );
        dataBytesOffset += NodeUUID::kBytesSize;
        stepObligationFlow = trustLineBalanceToBytes(value.second);
        memcpy(
                dataBytesShared.get() + dataBytesOffset,
                stepObligationFlow.data(),
                stepObligationFlow.size()
        );
        dataBytesOffset += stepObligationFlow.size();
        stepObligationFlow.clear();
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

    vector<byte> *stepObligationFlowBytes;
    NodeUUID stepNodeUUID;
    TrustLineBalance stepBalance;

    //  Get neighborsNodesCount
    uint16_t neighborsNodesCount;
    memcpy(
            &neighborsNodesCount,
            buffer.get() + bytesBufferOffset,
            sizeof(uint16_t)
    );
    bytesBufferOffset += sizeof(uint16_t);
//    Parse mNeighborsBalances
    for (uint16_t i=1; i<=neighborsNodesCount; i++){
        memcpy(
                stepNodeUUID.data,
                buffer.get() + bytesBufferOffset,
                NodeUUID::kBytesSize
        );
        bytesBufferOffset += NodeUUID::kBytesSize;
        stepObligationFlowBytes = new vector<byte>(
                buffer.get() + bytesBufferOffset,
                buffer.get() + bytesBufferOffset + kTrustLineBalanceSerializeBytesCount);
        stepBalance = bytesToTrustLineBalance(*stepObligationFlowBytes);
        mNeighborsBalances.push_back(make_pair(stepNodeUUID, stepBalance));
        bytesBufferOffset += kTrustLineBalanceSerializeBytesCount;
    };
}

const bool CyclesFourNodesBalancesResponseMessage::isTransactionMessage() const {
    return  true;
}

vector<pair<NodeUUID, TrustLineBalance>> CyclesFourNodesBalancesResponseMessage::NeighborsBalances() {
    return mNeighborsBalances;
}

void CyclesFourNodesBalancesResponseMessage::AddNeighborUUIDAndBalance(
    pair<NodeUUID, TrustLineBalance> neighborUUIDAndBalance) {
    mNeighborsBalances.push_back(neighborUUIDAndBalance);
}
