
#include "../../../common/Types.h"
#include "BoundaryNodeTopologyMessage.h"

BoundaryNodeTopologyMessage::BoundaryNodeTopologyMessage(const TrustLineBalance maxFlow,
                                                         const byte max_depth,
                                                         vector<NodeUUID> &path,
                                                         const vector<pair<NodeUUID, TrustLineBalance>> boundaryNodes) :

    InBetweenNodeTopologyMessage(
        maxFlow,
        max_depth,
        path
    ) {

    //TODO:: (D.V.) Init field before constructor's body. Example in InBetweenNodeTopologyMessage class-file.
    mBoundaryNodes = boundaryNodes;
}

BoundaryNodeTopologyMessage::BoundaryNodeTopologyMessage(
    BytesShared buffer) :

    InBetweenNodeTopologyMessage(buffer) {

    deserializeFromBytes(buffer);
}


//TODO:: (D.V.) How do you think, could somebody else quickly understand this code?
//TODO:: Please, format this.
pair<BytesShared, size_t> BoundaryNodeTopologyMessage::serializeToBytes() {
    auto parentBytesAndCount = InBetweenNodeTopologyMessage::serializeToBytes();
    size_t bytesBufferOffset = InBetweenNodeTopologyMessage::kOffsetToInheritedBytes();
    cout << "InBetweenNodeTopologyMessage serealize " << endl;
    cout << parentBytesAndCount.second << endl;
    cout << bytesBufferOffset << endl;
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
//    todo ask about type for count of first level nodes
//    for BoundaryNodes
//    uint16_t boundaryNodesCount = (uint16_t) mBoundaryNodes.size();
    memcpy(
      dataBytesShared.get() + dataBytesOffset,
      &boundaryNodesCount,
      sizeof(boundaryNodesCount)
    );
    dataBytesOffset += sizeof(boundaryNodesCount);
    vector<byte> stepObligationFlow;
    for(auto &value: mBoundaryNodes){
        cout << "Lets see what we have in value" << endl;
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
    cout << "InBetweenNodeTopologyMessage deserealize" << endl;
//    cout << parentBytesAndCount.second << endl;
    uint16_t boundaryNodesCount;
    memcpy(
            &boundaryNodesCount,
            buffer.get() + bytesBufferOffset,
            2
    );
    bytesBufferOffset += sizeof(boundaryNodesCount);

    vector<byte> stepObligationFlowBytes;
    NodeUUID stepNodeUUID;

    for (uint16_t i=1; i<=boundaryNodesCount; i++){
        memcpy(
                stepNodeUUID.data,
                buffer.get() + bytesBufferOffset,
                NodeUUID::kBytesSize
        );
        bytesBufferOffset += NodeUUID::kBytesSize;
        memcpy(
                &stepObligationFlowBytes,
                buffer.get() + bytesBufferOffset,
                kTrustLineBalanceSerializeBytesCount
        );
        mBoundaryNodes.push_back(make_pair(stepNodeUUID, bytesToTrustLineBalance(stepObligationFlowBytes)));
        bytesBufferOffset += kTrustLineBalanceSerializeBytesCount;
        stepObligationFlowBytes.clear();
    };
}