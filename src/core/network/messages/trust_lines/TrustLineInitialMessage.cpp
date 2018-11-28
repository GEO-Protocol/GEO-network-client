#include "TrustLineInitialMessage.h"

TrustLineInitialMessage::TrustLineInitialMessage(
    const SerializedEquivalent equivalent,
    const NodeUUID &sender,
    ContractorID idOnSenderSide,
    vector<BaseAddress::Shared> senderAddresses,
    const TransactionUUID &transactionUUID,
    const NodeUUID &destinationUUID,
    ContractorID contractorID,
    bool isContractorGateway)
    noexcept:

    DestinationMessage(
        equivalent,
        sender,
        idOnSenderSide,
        senderAddresses,
        transactionUUID,
        destinationUUID),
    mContractorID(contractorID),
    mIsContractorGateway(isContractorGateway)
{}

TrustLineInitialMessage::TrustLineInitialMessage(
    BytesShared buffer)
    noexcept :
    DestinationMessage(buffer)
{
    // todo: use desrializer

    size_t bytesBufferOffset = DestinationMessage::kOffsetToInheritedBytes();
    //----------------------------------------------------
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


const Message::MessageType TrustLineInitialMessage::typeID() const
    noexcept
{
    return Message::TrustLines_Initial;
}

const ContractorID TrustLineInitialMessage::contractorID() const
    noexcept
{
    return mContractorID;
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

    auto parentBytesAndCount = DestinationMessage::serializeToBytes();

    size_t bytesCount = parentBytesAndCount.second
                        + sizeof(ContractorID)
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
        &mContractorID,
        sizeof(ContractorID));
    dataBytesOffset += sizeof(ContractorID);
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