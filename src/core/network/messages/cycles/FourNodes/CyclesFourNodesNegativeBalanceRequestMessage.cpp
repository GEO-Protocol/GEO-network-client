#include "CyclesFourNodesNegativeBalanceRequestMessage.h"

CyclesFourNodesNegativeBalanceRequestMessage::CyclesFourNodesNegativeBalanceRequestMessage(
    const SerializedEquivalent equivalent,
    vector<BaseAddress::Shared> senderAddresses,
    const TransactionUUID &transactionUUID,
    BaseAddress::Shared contractorAddress,
    vector<BaseAddress::Shared> checkedNodes):

    TransactionMessage(
        equivalent,
        0,
        senderAddresses,
        transactionUUID),
    mContractorAddress(contractorAddress),
    mCheckedNodes(checkedNodes)
{}

CyclesFourNodesNegativeBalanceRequestMessage::CyclesFourNodesNegativeBalanceRequestMessage(
    BytesShared buffer):

    TransactionMessage(buffer)
{
    size_t bytesBufferOffset = TransactionMessage::kOffsetToInheritedBytes();
    // contractorAddress
    mContractorAddress = deserializeAddress(
        buffer.get() + bytesBufferOffset);
    bytesBufferOffset += mContractorAddress->serializedSize();

    // checkedNodes
    SerializedRecordsCount checkedNodesCount;
    memcpy(
        &checkedNodesCount,
        buffer.get() + bytesBufferOffset,
        sizeof(SerializedRecordsCount));
    bytesBufferOffset += sizeof(SerializedRecordsCount);

    for (SerializedRecordNumber i = 1; i <= checkedNodesCount; ++i) {
        auto stepAddress = deserializeAddress(
            buffer.get() + bytesBufferOffset);
        bytesBufferOffset += stepAddress->serializedSize();
        mCheckedNodes.push_back(stepAddress);
    }
}

pair<BytesShared, size_t> CyclesFourNodesNegativeBalanceRequestMessage::serializeToBytes() const
{
    auto parentBytesAndCount = TransactionMessage::serializeToBytes();

    auto debtorsCount = (SerializedRecordsCount)mCheckedNodes.size();
    size_t bytesCount = parentBytesAndCount.second
                        + mContractorAddress->serializedSize()
                        + sizeof(SerializedRecordsCount);
    for (const auto &address : mCheckedNodes) {
        bytesCount += address->serializedSize();
    }

    BytesShared dataBytesShared = tryCalloc(bytesCount);
    size_t dataBytesOffset = 0;
    //----------------------------------------------------
    memcpy(
        dataBytesShared.get(),
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second);
    dataBytesOffset += parentBytesAndCount.second;

    // For mContractor
    auto serializedContractorAddress = mContractorAddress->serializeToBytes();
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        serializedContractorAddress.get(),
        mContractorAddress->serializedSize());
    dataBytesOffset += mContractorAddress->serializedSize();

    // For mCheckedNodes
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        &debtorsCount,
        sizeof(SerializedRecordsCount));
    dataBytesOffset += sizeof(SerializedRecordsCount);

    for(auto const &address: mCheckedNodes) {
        auto serializedAddress = address->serializeToBytes();
        memcpy(
            dataBytesShared.get() + dataBytesOffset,
            serializedAddress.get(),
            address->serializedSize());
        dataBytesOffset += address->serializedSize();
    }

    return make_pair(
        dataBytesShared,
        bytesCount);
}

const Message::MessageType CyclesFourNodesNegativeBalanceRequestMessage::typeID() const
{
    return Message::MessageType::Cycles_FourNodesNegativeBalanceRequest;
}

vector<BaseAddress::Shared> CyclesFourNodesNegativeBalanceRequestMessage::checkedNodes() const
{
    return mCheckedNodes;
}

BaseAddress::Shared CyclesFourNodesNegativeBalanceRequestMessage::contractorAddress() const
{
    return mContractorAddress;
}