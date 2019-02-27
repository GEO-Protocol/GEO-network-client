#include "TrustLineConfirmationMessage.h"

TrustLineConfirmationMessage::TrustLineConfirmationMessage(
    const SerializedEquivalent equivalent,
    ContractorID idOnSenderSide,
    const TransactionUUID &transactionUUID,
    ContractorID contractorID,
    bool isContractorGateway,
    const OperationState state) :

    ConfirmationMessage(
        equivalent,
        idOnSenderSide,
        transactionUUID,
        state),
    mContractorID(contractorID),
    mIsContractorGateway(isContractorGateway)
{}

TrustLineConfirmationMessage::TrustLineConfirmationMessage(
    BytesShared buffer):

    ConfirmationMessage(buffer)
{
    size_t bytesBufferOffset = ConfirmationMessage::kOffsetToInheritedBytes();
    memcpy(
        &mContractorID,
        buffer.get() + bytesBufferOffset,
        sizeof(ContractorID));
    bytesBufferOffset += sizeof(ContractorID);
    //----------------------------------------------------
    memcpy(
        &mIsContractorGateway,
        buffer.get() + bytesBufferOffset,
        sizeof(byte));
}

const Message::MessageType TrustLineConfirmationMessage::typeID() const
{
    return Message::TrustLines_Confirmation;
}

const ContractorID TrustLineConfirmationMessage::contractorID() const
{
    return mContractorID;
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
            + sizeof(ContractorID)
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
        &mContractorID,
        sizeof(ContractorID));
    dataBytesOffset += sizeof(ContractorID);
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