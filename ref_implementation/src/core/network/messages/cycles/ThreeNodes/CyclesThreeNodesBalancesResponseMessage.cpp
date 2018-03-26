/**
 * This file is part of GEO Project.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Project/GEO-Project/blob/master/LICENSE.md
 *
 * No part of GEO Project, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#include "CyclesThreeNodesBalancesResponseMessage.h"


CyclesThreeNodesBalancesResponseMessage::CyclesThreeNodesBalancesResponseMessage(
    const NodeUUID &senderUUID,
    const TransactionUUID &transactionUUID,
    vector<NodeUUID> &neighbors) :
    TransactionMessage(senderUUID, transactionUUID),
    mNeighborsUUUID(neighbors)
{}

CyclesThreeNodesBalancesResponseMessage::CyclesThreeNodesBalancesResponseMessage(
    const NodeUUID &senderUUID,
    const TransactionUUID &transactionUUID,
    SerializedRecordsCount neighborsUUUIDCount):
    TransactionMessage(
        senderUUID,
        transactionUUID)
{
    mNeighborsUUUID.reserve(neighborsUUUIDCount);
}

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
    NodeUUID stepNodeUUID;
    for (SerializedRecordNumber i=1; i<=boundaryNodesCount; i++){
        memcpy(
            stepNodeUUID.data,
            buffer.get() + bytesBufferOffset,
            NodeUUID::kBytesSize);
        bytesBufferOffset += NodeUUID::kBytesSize;
        mNeighborsUUUID.push_back(stepNodeUUID);
    }
}

std::pair<BytesShared, size_t> CyclesThreeNodesBalancesResponseMessage::serializeToBytes() const
    throw (bad_alloc)
{
    auto parentBytesAndCount = TransactionMessage::serializeToBytes();

    SerializedRecordsCount boundaryNodesCount = (SerializedRecordsCount) mNeighborsUUUID.size();
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

vector<NodeUUID> CyclesThreeNodesBalancesResponseMessage::NeighborsAndBalances()
    noexcept
{
    return mNeighborsUUUID;
}

void CyclesThreeNodesBalancesResponseMessage::addNeighborUUIDAndBalance(
    NodeUUID neighborUUID)
    throw (bad_alloc)
{
    mNeighborsUUUID.push_back(neighborUUID);
}

