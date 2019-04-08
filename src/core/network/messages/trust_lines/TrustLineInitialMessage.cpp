#include "TrustLineInitialMessage.h"

TrustLineInitialMessage::TrustLineInitialMessage(
    const SerializedEquivalent equivalent,
    ContractorID idOnSenderSide,
    const TransactionUUID &transactionUUID,
    bool isContractorGateway)
    noexcept:

    TransactionMessage(
        equivalent,
        idOnSenderSide,
        transactionUUID),
    mIsContractorGateway(isContractorGateway)
{}

TrustLineInitialMessage::TrustLineInitialMessage(
    BytesShared buffer)
    noexcept :
    TransactionMessage(buffer)
{
    // todo: use deserializer

    size_t bytesBufferOffset = TransactionMessage::kOffsetToInheritedBytes();
    //----------------------------------------------------
    memcpy(
        &mIsContractorGateway,
        buffer.get() + bytesBufferOffset,
        sizeof(byte));
}


const Message::MessageType TrustLineInitialMessage::typeID() const
    noexcept
{
    return Message::TrustLines_Initial;
}

bool TrustLineInitialMessage::isEncrypted() const {
    return true;
}

const bool TrustLineInitialMessage::isContractorGateway() const
    noexcept
{
    return mIsContractorGateway;
}

const bool TrustLineInitialMessage::isCheckCachedResponse() const
{
    return true;
}

pair<BytesShared, size_t> TrustLineInitialMessage::serializeToBytes() const
{
    // todo: use serializer

    auto parentBytesAndCount = TransactionMessage::serializeToBytes();

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
