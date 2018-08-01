#include "SetIncomingTrustLineInitialMessage.h"

SetIncomingTrustLineInitialMessage::SetIncomingTrustLineInitialMessage(
    const SerializedEquivalent equivalent,
    const NodeUUID &sender,
    const TransactionUUID &transactionUUID,
    const NodeUUID &destination,
    const TrustLineAmount &amount,
    bool isContractorGateway)
    noexcept :

    SetIncomingTrustLineMessage(
        equivalent,
        sender,
        transactionUUID,
        destination,
        amount),
    mIsContractorGateway(isContractorGateway)
{}

SetIncomingTrustLineInitialMessage::SetIncomingTrustLineInitialMessage(
    BytesShared buffer)
    noexcept :
    SetIncomingTrustLineMessage(buffer)
{
    // todo: use desrializer

    size_t bytesBufferOffset = SetIncomingTrustLineMessage::kOffsetToInheritedBytes();
    //----------------------------------------------------
    memcpy(
        &mIsContractorGateway,
        buffer.get() + bytesBufferOffset,
        sizeof(byte));
}


const Message::MessageType SetIncomingTrustLineInitialMessage::typeID() const
    noexcept
{
    return Message::TrustLines_SetIncomingInitial;
}

const bool SetIncomingTrustLineInitialMessage::isContractorGateway() const
    noexcept
{
    return mIsContractorGateway;
}

pair<BytesShared, size_t> SetIncomingTrustLineInitialMessage::serializeToBytes() const
{
    // todo: use serializer

    auto parentBytesAndCount = SetIncomingTrustLineMessage::serializeToBytes();

    size_t bytesCount = parentBytesAndCount.second
                        + sizeof(byte);

    BytesShared dataBytesShared = tryCalloc(bytesCount);
    size_t dataBytesOffset = 0;
    //----------------------------------------------------
    memcpy(
        dataBytesShared.get(),
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second);
    dataBytesOffset += parentBytesAndCount.second;
    //----------------------------
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        &mIsContractorGateway,
        sizeof(byte));
    //----------------------------
    return make_pair(
        dataBytesShared,
        bytesCount);
}