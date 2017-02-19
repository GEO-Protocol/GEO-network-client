
#include "../../../common/Types.h"
#include "GetTopologyAndBalancesMessageBoundaryNode.h"

GetTopologyAndBalancesMessageBoundaryNode::GetTopologyAndBalancesMessageBoundaryNode(const TrustLineAmount maxFlow,
                                                                                     const byte max_depth,
                                                                                     vector<NodeUUID> &path,
                                                                                     const vector<pair<NodeUUID, TrustLineAmount>> boundaryNodes)
        : GetTopologyAndBalancesMessageInBetweenNode(maxFlow, max_depth, path) {
    mBoundaryNodes = boundaryNodes;
}

GetTopologyAndBalancesMessageBoundaryNode::GetTopologyAndBalancesMessageBoundaryNode(BytesShared buffer)
        : GetTopologyAndBalancesMessageInBetweenNode(buffer) {
    deserializeFromBytes(buffer);
}

pair<BytesShared, size_t> GetTopologyAndBalancesMessageBoundaryNode::serializeToBytes() {
    auto parentBytesAndCount = GetTopologyAndBalancesMessageInBetweenNode::serializeToBytes();

    size_t bytesCount =
            parentBytesAndCount.second + (NodeUUID::kBytesSize + kTrustLineAmountBytesCount) * mBoundaryNodes.size();

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
    uint16_t boundaryNodesCount = (uint16_t) mBoundaryNodes.size();
    memcpy(
      dataBytesShared.get() + dataBytesOffset,
      &boundaryNodesCount,
      sizeof(boundaryNodesCount)
    );
    dataBytesOffset += sizeof(boundaryNodesCount);
    vector<byte> stepObligationFlow;
    for(auto const &value: mBoundaryNodes){
        cout << "Lets see what we have in value" << endl;
    }
//        memcpy(
//                dataBytesShared.get() + dataBytesOffset,
//                &value.first,
//                NodeUUID::kBytesSize
//        );
//        dataBytesOffset += NodeUUID::kBytesSize;
//        stepObligationFlow = trustLineAmountToBytes(value.second);
//        memcpy(
//                dataBytesShared.get() + dataBytesOffset,
//                stepObligationFlow.data(),
//                stepObligationFlow.size()
//        );
//        dataBytesOffset += stepObligationFlow.size();
//        stepObligationFlow.clear();
//    }

    return make_pair(
            dataBytesShared,
            bytesCount
    );
}
void GetTopologyAndBalancesMessageBoundaryNode::deserializeFromBytes(BytesShared buffer) {
    GetTopologyAndBalancesMessageInBetweenNode::deserializeFromBytes(buffer);
    //    Parent part of deserializeFromBytes
//    size_t bytesBufferOffset = GetTopologyAndBalancesMessageInBetweenNode::kOffsetToInheritedBytes();
//    uint16_t boundaryNodesCount;
//    memcpy(
//            &boundaryNodesCount,
//            buffer.get() + bytesBufferOffset,
//            sizeof(boundaryNodesCount)
//    );
//    bytesBufferOffset += sizeof(boundaryNodesCount);
//
//    vector<byte> stepObligationFlowBytes;
//    NodeUUID stepNodeUUID;
//
//    for (uint16_t i=1; i<=boundaryNodesCount; i++){
//        memcpy(
//                stepNodeUUID.data,
//                buffer.get() + bytesBufferOffset,
//                NodeUUID::kBytesSize
//        );
//        bytesBufferOffset += NodeUUID::kBytesSize;
//        memcpy(
//                &stepObligationFlowBytes,
//                buffer.get(),
//                buffer.get() + kTrustLineAmountBytesCount
//        );
//        mBoundaryNodes.push_back(make_pair(stepNodeUUID, bytesToTrustLineAmount(stepObligationFlowBytes)));
//        bytesBufferOffset += kTrustLineAmountBytesCount;
//        stepObligationFlowBytes.clear();
//    };
}