#include "InitChannelMessage.h"

InitChannelMessage::InitChannelMessage(
    vector<BaseAddress::Shared> senderAddresses,
    const TransactionUUID &transactionUUID,
    Contractor::Shared contractor):

    TransactionMessage(
        0,
        senderAddresses,
        transactionUUID),
    mContractorID(contractor->getID()),
    mPublicKey(contractor->cryptoKey()->publicKey)
{
    encrypt(contractor);
}

InitChannelMessage::InitChannelMessage(
    BytesShared buffer):
    TransactionMessage(buffer)
{
    // todo: use deserializer

    size_t bytesBufferOffset = TransactionMessage::kOffsetToInheritedBytes();
    //----------------------------------------------------
    memcpy(
        &mContractorID,
        buffer.get() + bytesBufferOffset,
        sizeof(ContractorID));
    bytesBufferOffset += sizeof(ContractorID);

    mPublicKey = make_shared<MsgEncryptor::PublicKey>();
    memcpy(
        mPublicKey->key,
        buffer.get() + bytesBufferOffset,
        mPublicKey->kBytesSize);
}


const Message::MessageType InitChannelMessage::typeID() const
{
    return Message::Channel_Init;
}

const ContractorID InitChannelMessage::contractorID() const
{
    return mContractorID;
}

const MsgEncryptor::PublicKey::Shared InitChannelMessage::publicKey() const
{
    return mPublicKey;
}

pair<BytesShared, size_t> InitChannelMessage::serializeToBytes() const
{
    // todo: use serializer

    auto parentBytesAndCount = TransactionMessage::serializeToBytes();

    size_t bytesCount = parentBytesAndCount.second
                        + sizeof(ContractorID)
                        + mPublicKey->kBytesSize;

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
        &mPublicKey->key,
        mPublicKey->kBytesSize);
    //----------------------------
    return make_pair(
        dataBytesShared,
        bytesCount);
}

const bool InitChannelMessage::isAddToConfirmationRequiredMessagesHandler() const
{
    return true;
}