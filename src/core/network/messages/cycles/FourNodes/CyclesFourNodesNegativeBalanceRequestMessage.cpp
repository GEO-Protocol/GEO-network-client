#include "CyclesFourNodesNegativeBalanceRequestMessage.h"

CyclesFourNodesNegativeBalanceRequestMessage::CyclesFourNodesNegativeBalanceRequestMessage(
    const SerializedEquivalent equivalent,
    const NodeUUID &senderUUID,
    const TransactionUUID &transactionUUID,
    const NodeUUID &contractor,
    vector<NodeUUID> &checkedNodes):

    TransactionMessage(
        equivalent,
        senderUUID,
        transactionUUID),
    mContractorUUID(contractor),
    mCheckedNodes(checkedNodes)
{}

CyclesFourNodesNegativeBalanceRequestMessage::CyclesFourNodesNegativeBalanceRequestMessage(
    BytesShared buffer):

    TransactionMessage(buffer)
{
    size_t bytesBufferOffset = TransactionMessage::kOffsetToInheritedBytes();
    // contractorUUID
    memcpy(
        mContractorUUID.data,
        buffer.get() + bytesBufferOffset,
        NodeUUID::kBytesSize);
    bytesBufferOffset += NodeUUID::kBytesSize;

    // checkedNodes
    SerializedRecordsCount checkedNodesCount;
    memcpy(
        &checkedNodesCount,
        buffer.get() + bytesBufferOffset,
        sizeof(SerializedRecordsCount));
    bytesBufferOffset += sizeof(SerializedRecordsCount);

    for (SerializedRecordNumber i = 1; i <= checkedNodesCount; ++i) {
        NodeUUID stepNode(buffer.get() + bytesBufferOffset);
        bytesBufferOffset += NodeUUID::kBytesSize;
        mCheckedNodes.push_back(stepNode);
    }
}

pair<BytesShared, size_t> CyclesFourNodesNegativeBalanceRequestMessage::serializeToBytes() const
    throw(bad_alloc)
{
    auto parentBytesAndCount = TransactionMessage::serializeToBytes();

    auto debtorsCount = (SerializedRecordsCount)mCheckedNodes.size();
    size_t bytesCount = parentBytesAndCount.second
                        + NodeUUID::kBytesSize
                        + sizeof(SerializedRecordsCount)
                        + debtorsCount * NodeUUID::kBytesSize;

    BytesShared dataBytesShared = tryCalloc(bytesCount);
    size_t dataBytesOffset = 0;
    //----------------------------------------------------
    memcpy(
        dataBytesShared.get(),
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second);
    dataBytesOffset += parentBytesAndCount.second;

    // For mContractor
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        &mContractorUUID,
        NodeUUID::kBytesSize);
    dataBytesOffset += NodeUUID::kBytesSize;

    // For mCheckedNodes
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        &debtorsCount,
        sizeof(SerializedRecordsCount));
    dataBytesOffset += sizeof(SerializedRecordsCount);

    for(auto const &debtor: mCheckedNodes) {
        memcpy(
            dataBytesShared.get() + dataBytesOffset,
            &debtor,
            NodeUUID::kBytesSize);
        dataBytesOffset += NodeUUID::kBytesSize;
    }

    return make_pair(
        dataBytesShared,
        bytesCount);
}

const Message::MessageType CyclesFourNodesNegativeBalanceRequestMessage::typeID() const
{
    return Message::MessageType::Cycles_FourNodesNegativeBalanceRequest;
}

vector<NodeUUID> CyclesFourNodesNegativeBalanceRequestMessage::checkedNodes() const
{
    return mCheckedNodes;
}

const NodeUUID CyclesFourNodesNegativeBalanceRequestMessage::contractor() const
{
    return mContractorUUID;
}