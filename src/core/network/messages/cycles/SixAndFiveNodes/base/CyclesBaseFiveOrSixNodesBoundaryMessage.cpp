#include "CyclesBaseFiveOrSixNodesBoundaryMessage.h"

CyclesBaseFiveOrSixNodesBoundaryMessage::CyclesBaseFiveOrSixNodesBoundaryMessage(
        vector<NodeUUID>& path,
        vector<NodeUUID>& boundaryNodes) :

    CycleBaseFiveOrSixNodesInBetweenMessage(
            path
    ),
    mBoundaryNodes(boundaryNodes)
{
}

CyclesBaseFiveOrSixNodesBoundaryMessage::CyclesBaseFiveOrSixNodesBoundaryMessage(BytesShared buffer)
{
    deserializeFromBytes(buffer);
}

pair<BytesShared, size_t> CyclesBaseFiveOrSixNodesBoundaryMessage::serializeToBytes() {
    auto parentBytesAndCount = CycleBaseFiveOrSixNodesInBetweenMessage::serializeToBytes();

    uint16_t boundaryNodesCount = (uint16_t) mBoundaryNodes.size();
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
//    for BoundaryNodes
    memcpy(
      dataBytesShared.get() + dataBytesOffset,
      &boundaryNodesCount,
      sizeof(uint16_t)
    );
    dataBytesOffset += sizeof(uint16_t);
    vector<byte> stepObligationFlow;
    for(const auto &kNodeUUID: mBoundaryNodes){
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
void CyclesBaseFiveOrSixNodesBoundaryMessage::deserializeFromBytes(BytesShared buffer) {
    CycleBaseFiveOrSixNodesInBetweenMessage::deserializeFromBytes(buffer);
//        Parent part of deserializeFromBytes
    size_t bytesBufferOffset = CycleBaseFiveOrSixNodesInBetweenMessage::kOffsetToInheritedBytes();
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
        mBoundaryNodes.push_back(stepNodeUUID);
    };
}

const bool CyclesBaseFiveOrSixNodesBoundaryMessage::isCyclesDiscoveringResponseMessage() const {
    return true;
}

const vector<NodeUUID> CyclesBaseFiveOrSixNodesBoundaryMessage::BoundaryNodes() const {
    return mBoundaryNodes;
}