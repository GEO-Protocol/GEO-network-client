#include "FourNodesBalancesResponseMessage.h"

FourNodesBalancesResponseMessage::FourNodesBalancesResponseMessage(const TrustLineBalance &maxFlow,
                                                                   vector<pair<NodeUUID, TrustLineBalance>> &neighborsBalancesCreditors,
                                                                   vector<pair<NodeUUID, TrustLineBalance>> &neighborsBalancesDebtors):
        mNeighborsBalancesCreditors(neighborsBalancesCreditors),
        mNeighborsBalancesDebtors(neighborsBalancesDebtors),
        mMaxFlow(maxFlow)
{

}

std::pair<BytesShared, size_t> FourNodesBalancesResponseMessage::serializeToBytes() {
    auto parentBytesAndCount = TransactionMessage::serializeToBytes();

    uint16_t neighborsDebtorsNodesCount = (uint16_t) mNeighborsBalancesDebtors.size();
    uint16_t neighborsCreditorsNodesCount = (uint16_t) mNeighborsBalancesCreditors.size();
    size_t bytesCount =
            parentBytesAndCount.second +
            (NodeUUID::kBytesSize + kTrustLineBalanceSerializeBytesCount) * neighborsCreditorsNodesCount +
            (NodeUUID::kBytesSize + kTrustLineBalanceSerializeBytesCount) * neighborsDebtorsNodesCount +
            sizeof(neighborsCreditorsNodesCount) +
            sizeof(neighborsDebtorsNodesCount);

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
//    for mNeighborsCreditorsNodes
    memcpy(
            dataBytesShared.get() + dataBytesOffset,
            &neighborsCreditorsNodesCount,
            sizeof(uint16_t)
    );
    dataBytesOffset += sizeof(uint16_t);
    vector<byte> stepObligationFlow;
    for(auto &value: mNeighborsBalancesCreditors){
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
// for mNeighborsBalancesDebtors
    memcpy(
            dataBytesShared.get() + dataBytesOffset,
            &neighborsDebtorsNodesCount,
            sizeof(uint16_t)
    );
    dataBytesOffset += sizeof(uint16_t);
    for(auto &value: mNeighborsBalancesDebtors){
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

const Message::MessageType FourNodesBalancesResponseMessage::typeID() const {
    return Message::MessageTypeID::FourNodesBalancesResponseMessage;
}

FourNodesBalancesResponseMessage::FourNodesBalancesResponseMessage(BytesShared buffer) {
    deserializeFromBytes(buffer);
}

void FourNodesBalancesResponseMessage::deserializeFromBytes(BytesShared buffer) {
    TransactionMessage::deserializeFromBytes(buffer);
    size_t bytesBufferOffset = TransactionMessage::kOffsetToInheritedBytes();

    vector<byte> *stepObligationFlowBytes;
    NodeUUID stepNodeUUID;
    TrustLineBalance stepBalance;

    //    Get neighborsDebtorsNodesCount
    uint16_t neighborsCreditorsNodesCount;
    memcpy(
            &neighborsCreditorsNodesCount,
            buffer.get() + bytesBufferOffset,
            sizeof(uint16_t)
    );
    bytesBufferOffset += sizeof(uint16_t);
//  Parse mNeighborsBalancesCreditors
    for (uint16_t i=1; i<=neighborsCreditorsNodesCount; i++){
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
        mNeighborsBalancesCreditors.push_back(make_pair(stepNodeUUID, stepBalance));
        bytesBufferOffset += kTrustLineBalanceSerializeBytesCount;
    };


    //    Get neighborsDebtorsNodesCount
    uint16_t neighborsDebtorsNodesCount;
    memcpy(
            &neighborsDebtorsNodesCount,
            buffer.get() + bytesBufferOffset,
            sizeof(uint16_t)
    );
    bytesBufferOffset += sizeof(uint16_t);
//    Parse mNeighborsBalancesDebtors
    for (uint16_t i=1; i<=neighborsDebtorsNodesCount; i++){
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
        mNeighborsBalancesDebtors.push_back(make_pair(stepNodeUUID, stepBalance));
        bytesBufferOffset += kTrustLineBalanceSerializeBytesCount;
    };
}

const bool FourNodesBalancesResponseMessage::isTransactionMessage() const {
    return  true;
}

vector<pair<NodeUUID, TrustLineBalance>> FourNodesBalancesResponseMessage::NeighborsBalancesDebtors() {
    return mNeighborsBalancesDebtors;
}

vector<pair<NodeUUID, TrustLineBalance>> FourNodesBalancesResponseMessage::NeighborsBalancesCreditors() {
    return mNeighborsBalancesCreditors;
}
