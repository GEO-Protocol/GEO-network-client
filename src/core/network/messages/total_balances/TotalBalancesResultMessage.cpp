#include "TotalBalancesResultMessage.h"

TotalBalancesResultMessage::TotalBalancesResultMessage(
    const NodeUUID& senderUUID,
    const TransactionUUID &transactionUUID,
    const TrustLineAmount &totalIncomingTrust,
    const TrustLineAmount &totalTrustUsedByContractor,
    const TrustLineAmount &totalOutgoingTrust,
    const TrustLineAmount &totalTrustUsedBySelf):

    TransactionMessage(
        senderUUID,
        transactionUUID),
    mTotalIncomingTrust(totalIncomingTrust),
    mTotalTrustUsedByContractor(totalTrustUsedByContractor),
    mTotalOutgoingTrust(totalOutgoingTrust),
    mTotalTrustUsedBySelf(totalTrustUsedBySelf)
{}

TotalBalancesResultMessage::TotalBalancesResultMessage(
    BytesShared buffer):

    TransactionMessage(buffer)
{
    size_t bytesBufferOffset = TransactionMessage::kOffsetToInheritedBytes();
    //----------------------------------------------------
    vector<byte> bufferTotalIncomingTrust(
        buffer.get() + bytesBufferOffset,
        buffer.get() + bytesBufferOffset + kTrustLineAmountBytesCount);
    bytesBufferOffset += kTrustLineAmountBytesCount;
    mTotalIncomingTrust = bytesToTrustLineAmount(bufferTotalIncomingTrust);
    //-----------------------------------------------------
    vector<byte> bufferTotalTrustUsedByContractor(
        buffer.get() + bytesBufferOffset,
        buffer.get() + bytesBufferOffset + kTrustLineAmountBytesCount);
    bytesBufferOffset += kTrustLineAmountBytesCount;
    mTotalTrustUsedByContractor = bytesToTrustLineAmount(bufferTotalTrustUsedByContractor);
    //-----------------------------------------------------
    vector<byte> bufferTotalOutgoingTrust(
        buffer.get() + bytesBufferOffset,
        buffer.get() + bytesBufferOffset + kTrustLineAmountBytesCount);
    bytesBufferOffset += kTrustLineAmountBytesCount;
    mTotalOutgoingTrust = bytesToTrustLineAmount(bufferTotalOutgoingTrust);
    //-----------------------------------------------------
    vector<byte> bufferTotalTrustUsedBySelf(
        buffer.get() + bytesBufferOffset,
        buffer.get() + bytesBufferOffset + kTrustLineAmountBytesCount);
    mTotalTrustUsedBySelf = bytesToTrustLineAmount(bufferTotalTrustUsedBySelf);

}

const Message::MessageType TotalBalancesResultMessage::typeID() const
{
    return Message::MessageType::TotalBalance_Response;
}

pair<BytesShared, size_t> TotalBalancesResultMessage::serializeToBytes() const
    throw(bad_alloc)
{
    auto parentBytesAndCount = TransactionMessage::serializeToBytes();
    size_t bytesCount = parentBytesAndCount.second + 4 * kTrustLineAmountBytesCount;
    BytesShared dataBytesShared = tryCalloc(bytesCount);
    size_t dataBytesOffset = 0;
    //----------------------------------------------------
    memcpy(
        dataBytesShared.get(),
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second);
    dataBytesOffset += parentBytesAndCount.second;
    //----------------------------------------------------
    vector<byte> buffer = trustLineAmountToBytes(mTotalIncomingTrust);
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        buffer.data(),
        buffer.size());
    dataBytesOffset += kTrustLineAmountBytesCount;
    //----------------------------------------------------
    buffer = trustLineAmountToBytes(mTotalTrustUsedByContractor);
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        buffer.data(),
        buffer.size());
    dataBytesOffset += kTrustLineAmountBytesCount;
    //----------------------------------------------------
    buffer = trustLineAmountToBytes(mTotalOutgoingTrust);
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        buffer.data(),
        buffer.size());
    dataBytesOffset += kTrustLineAmountBytesCount;
    //----------------------------------------------------
    buffer = trustLineAmountToBytes(mTotalTrustUsedBySelf);
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        buffer.data(),
        buffer.size());
    //----------------------------------------------------
    return make_pair(
        dataBytesShared,
        bytesCount);
}

const TrustLineAmount& TotalBalancesResultMessage::totalIncomingTrust() const
{
    return mTotalIncomingTrust;
}

const TrustLineAmount& TotalBalancesResultMessage::totalTrustUsedByContractor() const
{
    return mTotalTrustUsedByContractor;
}

const TrustLineAmount& TotalBalancesResultMessage::totalOutgoingTrust() const
{
    return mTotalOutgoingTrust;
}

const TrustLineAmount& TotalBalancesResultMessage::totalTrustUsedBySelf() const
{
    return mTotalTrustUsedBySelf;
}
