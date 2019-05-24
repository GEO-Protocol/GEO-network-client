#include "TrustLineInitialMessage.h"

TrustLineInitialMessage::TrustLineInitialMessage(
    const SerializedEquivalent equivalent,
    Contractor::Shared contractor,
    const TransactionUUID &transactionUUID,
    bool isContractorGateway):

    TransactionMessage(
        equivalent,
        contractor->ownIdOnContractorSide(),
        transactionUUID),
    mIsContractorGateway(isContractorGateway)
{
    encrypt(contractor);
}

TrustLineInitialMessage::TrustLineInitialMessage(
    BytesShared buffer):
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
{
    return Message::TrustLines_Initial;
}

const bool TrustLineInitialMessage::isContractorGateway() const
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
