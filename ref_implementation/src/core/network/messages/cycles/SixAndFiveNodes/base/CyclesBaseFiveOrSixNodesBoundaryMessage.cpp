/**
 * This file is part of GEO Protocol.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Protocol/GEO-network-client/blob/master/LICENSE.md
 *
 * No part of GEO Protocol, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#include "CyclesBaseFiveOrSixNodesBoundaryMessage.h"

CyclesBaseFiveOrSixNodesBoundaryMessage::CyclesBaseFiveOrSixNodesBoundaryMessage(
    vector<NodeUUID>& path,
    vector<NodeUUID>& boundaryNodes) :

    CycleBaseFiveOrSixNodesInBetweenMessage(path),
    mBoundaryNodes(boundaryNodes)
{}

CyclesBaseFiveOrSixNodesBoundaryMessage::CyclesBaseFiveOrSixNodesBoundaryMessage(BytesShared buffer)
{
    deserializeFromBytes(buffer);
}

pair<BytesShared, size_t> CyclesBaseFiveOrSixNodesBoundaryMessage::serializeToBytes() const
    throw(bad_alloc)
{
    auto parentBytesAndCount = CycleBaseFiveOrSixNodesInBetweenMessage::serializeToBytes();

    SerializedRecordsCount boundaryNodesCount = (SerializedRecordsCount) mBoundaryNodes.size();
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
    //    for BoundaryNodes
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        &boundaryNodesCount,
        sizeof(SerializedRecordsCount));
    dataBytesOffset += sizeof(SerializedRecordsCount);
    vector<byte> stepObligationFlow;
    for(const auto &kNodeUUID: mBoundaryNodes){
        memcpy(
            dataBytesShared.get() + dataBytesOffset,
            &kNodeUUID,
            NodeUUID::kBytesSize);
        dataBytesOffset += NodeUUID::kBytesSize;
    }
    return make_pair(
        dataBytesShared,
        bytesCount);
}

void CyclesBaseFiveOrSixNodesBoundaryMessage::deserializeFromBytes(BytesShared buffer)
{
    CycleBaseFiveOrSixNodesInBetweenMessage::deserializeFromBytes(buffer);
    size_t bytesBufferOffset = CycleBaseFiveOrSixNodesInBetweenMessage::kOffsetToInheritedBytes();
    //    Get NodesCount
    SerializedRecordsCount boundaryNodesCount;
    memcpy(
        &boundaryNodesCount,
        buffer.get() + bytesBufferOffset,
        sizeof(SerializedRecordsCount));
    bytesBufferOffset += sizeof(SerializedRecordsCount);
    //    Parse boundary nodes
    NodeUUID stepNodeUUID;
    for (SerializedRecordNumber i=1; i<=boundaryNodesCount; i++){
        memcpy(
            stepNodeUUID.data,
            buffer.get() + bytesBufferOffset,
            NodeUUID::kBytesSize);
        bytesBufferOffset += NodeUUID::kBytesSize;
        mBoundaryNodes.push_back(stepNodeUUID);
    }
}

const vector<NodeUUID> CyclesBaseFiveOrSixNodesBoundaryMessage::BoundaryNodes() const
{
    return mBoundaryNodes;
}
