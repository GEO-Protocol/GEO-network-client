#include "CloseTrustLineMessage.h"


CloseTrustLineMessage::CloseTrustLineMessage(
    const NodeUUID &sender,
    const TransactionUUID &transactionUUID,
    const NodeUUID &contractorUUID)
    noexcept :

    TransactionMessage(
        sender,
        transactionUUID),
    mContractorUUID(contractorUUID)
{}

const Message::MessageType CloseTrustLineMessage::typeID() const
    noexcept
{
    return Message::MessageType::TrustLines_Close;
}

/*
 * ToDo: rewrite me with bytes deserializer
 */
pair<BytesShared, size_t> CloseTrustLineMessage::serializeToBytes() const
    throw (bad_alloc)
{
    auto parentBytesAndCount = TransactionMessage::serializeToBytes();

    size_t bytesCount = parentBytesAndCount.second
                        + NodeUUID::kBytesSize;

    BytesShared dataBytesShared = tryCalloc(bytesCount);
    size_t dataBytesOffset = 0;
    //----------------------------------------------------
    memcpy(
        dataBytesShared.get(),
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second
    );
    dataBytesOffset += parentBytesAndCount.second;
    //----------------------------------------------------
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        mContractorUUID.data,
        NodeUUID::kBytesSize
    );
    //----------------------------
    return make_pair(
        dataBytesShared,
        bytesCount
    );
}