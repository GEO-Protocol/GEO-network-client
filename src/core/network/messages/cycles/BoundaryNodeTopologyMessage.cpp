
#include "../../../common/Types.h"
#include "BoundaryNodeTopologyMessage.h"

BoundaryNodeTopologyMessage::BoundaryNodeTopologyMessage(
        const CycleType cycleType,
        const TrustLineBalance &maxFlow,
        const byte &max_depth,
        vector<NodeUUID> &path,
        const vector<pair<NodeUUID, TrustLineBalance>> &boundaryNodes) :

    InBetweenNodeTopologyMessage(
        cycleType,
        maxFlow,
        max_depth,
        path
    ),
    mBoundaryNodes(boundaryNodes)
{
}

BoundaryNodeTopologyMessage::BoundaryNodeTopologyMessage(BytesShared buffer)
{
    deserializeFromBytes(buffer);
}


pair<BytesShared, size_t> BoundaryNodeTopologyMessage::serializeToBytes() {
    auto parentBytesAndCount = InBetweenNodeTopologyMessage::serializeToBytes();

    uint16_t boundaryNodesCount = (uint16_t) mBoundaryNodes.size();
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
//    for BoundaryNodes
    memcpy(
      dataBytesShared.get() + dataBytesOffset,
      &boundaryNodesCount,
      sizeof(uint16_t)
    );
    dataBytesOffset += sizeof(uint16_t);
    vector<byte> stepObligationFlow;
    for(auto &value: mBoundaryNodes){
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
void BoundaryNodeTopologyMessage::deserializeFromBytes(BytesShared buffer) {
    InBetweenNodeTopologyMessage::deserializeFromBytes(buffer);
//        Parent part of deserializeFromBytes
    size_t bytesBufferOffset = InBetweenNodeTopologyMessage::kOffsetToInheritedBytes();
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
        mBoundaryNodes.push_back(make_pair(stepNodeUUID, stepBalance));
        bytesBufferOffset += kTrustLineBalanceSerializeBytesCount;
    };
}

const bool BoundaryNodeTopologyMessage::isCyclesDiscoveringResponseMessage() const {
    return true;
}

const vector<pair<NodeUUID, TrustLineBalance>> BoundaryNodeTopologyMessage::BoundaryNodes() const {
    return mBoundaryNodes;
}

const Message::MessageType BoundaryNodeTopologyMessage::typeID() const {
    return Message::BoundaryNodeTopologyMessage;
}
