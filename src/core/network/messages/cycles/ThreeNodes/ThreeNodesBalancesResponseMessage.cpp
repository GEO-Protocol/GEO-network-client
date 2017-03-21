#include "ThreeNodesBalancesResponseMessage.h"

std::pair<BytesShared, size_t> ThreeNodesBalancesResponseMessage::serializeToBytes() {
    auto parentBytesAndCount = TransactionMessage::serializeToBytes();

    uint16_t boundaryNodesCount = (uint16_t) mNeighborsBalances.size();
    size_t bytesCount =
            parentBytesAndCount.second +
            (NodeUUID::kBytesSize + kTrustLineBalanceSerializeBytesCount) * boundaryNodesCount +
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
    vector<byte> stepObligationFlow;
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

const Message::MessageType ThreeNodesBalancesResponseMessage::typeID() const {
    return Message::MessageTypeID::ThreeNodesBalancesResponseMessage;
}

ThreeNodesBalancesResponseMessage::ThreeNodesBalancesResponseMessage() {

}

ThreeNodesBalancesResponseMessage::ThreeNodesBalancesResponseMessage(const TrustLineBalance &maxFlow,
                                                 vector<pair<NodeUUID, TrustLineBalance>> &neighbors) {

}

ThreeNodesBalancesResponseMessage::ThreeNodesBalancesResponseMessage(BytesShared buffer) {
    deserializeFromBytes(buffer);
}

void ThreeNodesBalancesResponseMessage::deserializeFromBytes(BytesShared buffer) {
    SenderMessage::deserializeFromBytes(buffer);
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
    vector<byte> *stepObligationFlowBytes;
    NodeUUID stepNodeUUID;
    TrustLineBalance stepBalance;
    for (uint16_t i=1; i<=boundaryNodesCount; i++){
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

const size_t ThreeNodesBalancesResponseMessage::kOffsetToInheritedBytes() {
    return 0;
}

vector<pair<NodeUUID, TrustLineBalance>> ThreeNodesBalancesResponseMessage::NeighborsAndBalances() {
    return mNeighborsBalances;
}

const bool ThreeNodesBalancesResponseMessage::isTransactionMessage() const {
    return  true;
}
