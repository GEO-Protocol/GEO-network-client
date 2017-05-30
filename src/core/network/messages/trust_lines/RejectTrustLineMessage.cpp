#include "RejectTrustLineMessage.h"


RejectTrustLineMessage::RejectTrustLineMessage(
    BytesShared buffer)
    noexcept :

    TransactionMessage(buffer)
{
    BytesDeserializer deserializer(
        buffer,
        TransactionMessage::kOffsetToInheritedBytes());

    deserializer.copyIntoDespiteConst(&mContractorUUID);
}

const Message::MessageType RejectTrustLineMessage::typeID() const
    noexcept
{
    return Message::MessageType::TrustLines_Reject;
}

const NodeUUID &RejectTrustLineMessage::contractorUUID() const
    noexcept
{
    return mContractorUUID;
}

/*
 * ToDo: rewrite me with bytes deserializer
 */
pair<BytesShared, size_t> RejectTrustLineMessage::serializeToBytes() const
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

// todo: refactor me
MessageResult::SharedConst RejectTrustLineMessage::resultRejected()
    throw (bad_alloc)
{

    return MessageResult::SharedConst(
        new MessageResult(
            senderUUID,
            mTransactionUUID,
            kResultCodeRejected)
    );
}
