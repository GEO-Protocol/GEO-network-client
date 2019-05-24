#include "TrustLineConfirmationMessage.h"

TrustLineConfirmationMessage::TrustLineConfirmationMessage(
    const SerializedEquivalent equivalent,
    Contractor::Shared contractor,
    const TransactionUUID &transactionUUID,
    bool isContractorGateway,
    const OperationState state) :

    ConfirmationMessage(
        equivalent,
        contractor->ownIdOnContractorSide(),
        transactionUUID,
        state),
    mIsContractorGateway(isContractorGateway)
{
    encrypt(contractor);
}

TrustLineConfirmationMessage::TrustLineConfirmationMessage(
    BytesShared buffer):

    ConfirmationMessage(buffer)
{
    size_t bytesBufferOffset = ConfirmationMessage::kOffsetToInheritedBytes();
    memcpy(
        &mIsContractorGateway,
        buffer.get() + bytesBufferOffset,
        sizeof(byte));
}

const Message::MessageType TrustLineConfirmationMessage::typeID() const
{
    return Message::TrustLines_Confirmation;
}

const bool TrustLineConfirmationMessage::isContractorGateway() const
{
    return mIsContractorGateway;
}

pair<BytesShared, size_t> TrustLineConfirmationMessage::serializeToBytes() const
{
    auto parentBytesAndCount = ConfirmationMessage::serializeToBytes();

    size_t bytesCount =
            parentBytesAndCount.second
            + sizeof(byte);

    BytesShared dataBytesShared = tryMalloc(bytesCount);
    size_t dataBytesOffset = 0;
    //----------------------------------------------------
    memcpy(
        dataBytesShared.get(),
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second);
    dataBytesOffset += parentBytesAndCount.second;
    //----------------------------------------------------
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        &mIsContractorGateway,
        sizeof(byte));
    //----------------------------------------------------
    return make_pair(
        dataBytesShared,
        bytesCount);
}