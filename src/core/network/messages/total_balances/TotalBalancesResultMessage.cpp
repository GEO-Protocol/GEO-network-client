#include "TotalBalancesResultMessage.h"

TotalBalancesResultMessage::TotalBalancesResultMessage(
    const NodeUUID& senderUUID,
    const TransactionUUID &transactionUUID,
    const TrustLineAmount &totalIncomingTrust,
    const TrustLineAmount &totalIncomingTrustUsed,
    const TrustLineAmount &totalOutgoingTrust,
    const TrustLineAmount &totalOutgoingTrustUsed):

    TransactionMessage(
        senderUUID,
        transactionUUID),
    mTotalIncomingTrust(totalIncomingTrust),
    mTotalIncomingTrustUsed(totalIncomingTrustUsed),
    mTotalOutgoingTrust(totalOutgoingTrust),
    mTotalOutgoingTrustUsed(totalOutgoingTrustUsed) {}


TotalBalancesResultMessage::TotalBalancesResultMessage(
        BytesShared buffer) {

    deserializeFromBytes(buffer);
}

const Message::MessageType TotalBalancesResultMessage::typeID() const {
    return Message::MessageTypeID::TotalBalancesResultMessageType;
}

pair<BytesShared, size_t> TotalBalancesResultMessage::serializeToBytes() {

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
    buffer = trustLineAmountToBytes(mTotalIncomingTrust);
    memcpy(
            dataBytesShared.get() + dataBytesOffset,
            buffer.data(),
            buffer.size());
    dataBytesOffset += kTrustLineAmountBytesCount;
    //----------------------------------------------------
    buffer = trustLineAmountToBytes(mTotalIncomingTrust);
    memcpy(
            dataBytesShared.get() + dataBytesOffset,
            buffer.data(),
            buffer.size());
    dataBytesOffset += kTrustLineAmountBytesCount;
    //----------------------------------------------------
    buffer = trustLineAmountToBytes(mTotalIncomingTrust);
    memcpy(
            dataBytesShared.get() + dataBytesOffset,
            buffer.data(),
            buffer.size());
    dataBytesOffset += kTrustLineAmountBytesCount;
    //----------------------------------------------------
    return make_pair(
            dataBytesShared,
            bytesCount);
}

void TotalBalancesResultMessage::deserializeFromBytes(
        BytesShared buffer){

    TransactionMessage::deserializeFromBytes(buffer);
    size_t bytesBufferOffset = TransactionMessage::kOffsetToInheritedBytes();
    //----------------------------------------------------
    vector<byte> bufferTotalIncomingTrust(
            buffer.get() + bytesBufferOffset,
            buffer.get() + bytesBufferOffset + kTrustLineAmountBytesCount);
    bytesBufferOffset += kTrustLineAmountBytesCount;
    mTotalIncomingTrust = bytesToTrustLineAmount(bufferTotalIncomingTrust);
    //-----------------------------------------------------
    vector<byte> bufferTotalIncomingTrustUsed(
            buffer.get() + bytesBufferOffset,
            buffer.get() + bytesBufferOffset + kTrustLineAmountBytesCount);
    bytesBufferOffset += kTrustLineAmountBytesCount;
    mTotalIncomingTrustUsed = bytesToTrustLineAmount(bufferTotalIncomingTrustUsed);
    //-----------------------------------------------------
    vector<byte> bufferTotalOutgoingTrust(
            buffer.get() + bytesBufferOffset,
            buffer.get() + bytesBufferOffset + kTrustLineAmountBytesCount);
    bytesBufferOffset += kTrustLineAmountBytesCount;
    mTotalOutgoingTrust = bytesToTrustLineAmount(bufferTotalOutgoingTrust);
    //-----------------------------------------------------
    vector<byte> bufferTotalOutgoingTrustUsed(
            buffer.get() + bytesBufferOffset,
            buffer.get() + bytesBufferOffset + kTrustLineAmountBytesCount);
    bytesBufferOffset += kTrustLineAmountBytesCount;
    mTotalOutgoingTrustUsed = bytesToTrustLineAmount(bufferTotalOutgoingTrustUsed);
    //-----------------------------------------------------
}

const bool TotalBalancesResultMessage::isTotalBalancesResponseMessage() const {
    return true;
}

const TrustLineAmount& TotalBalancesResultMessage::totalIncomingTrust() const {
    return mTotalIncomingTrust;
}

const TrustLineAmount& TotalBalancesResultMessage::totalIncomingTrustUsed() const {
    return mTotalIncomingTrustUsed;
}

const TrustLineAmount& TotalBalancesResultMessage::totalOutgoingTrust() const {
    return mTotalOutgoingTrust;
}

const TrustLineAmount& TotalBalancesResultMessage::totalOutgoingTrustUsed() const {
    return mTotalOutgoingTrustUsed;
}
