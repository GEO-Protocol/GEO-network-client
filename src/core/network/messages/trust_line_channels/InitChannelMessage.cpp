#include "InitChannelMessage.h"

InitChannelMessage::InitChannelMessage(
    vector<BaseAddress::Shared> senderAddresses,
    const TransactionUUID &transactionUUID,
    Contractor &contractor)
    noexcept:

    TransactionMessage(
        0,
        senderAddresses,
        transactionUUID),
    mContractorID(contractor.getID()),
    mPublicKey(contractor.cryptoKey().publicKey)
{
    encrypt(mContractorID);
}

InitChannelMessage::InitChannelMessage(
    BytesShared buffer)
    noexcept :
    TransactionMessage(buffer)
{
    // todo: use deserializer

    size_t bytesBufferOffset = TransactionMessage::kOffsetToInheritedBytes();
    //----------------------------------------------------
    memcpy(
        &mContractorID,
        buffer.get() + bytesBufferOffset,
        sizeof(ContractorID));

    mPublicKey = make_shared<MsgEncryptor::PublicKey>();
    memcpy(
        mPublicKey->key,
        buffer.get() + bytesBufferOffset + sizeof(ContractorID),
        sizeof(mPublicKey->key));
}


const Message::MessageType InitChannelMessage::typeID() const
noexcept
{
    return Message::Channel_Init;
}

const ContractorID InitChannelMessage::contractorID() const
noexcept
{
    return mContractorID;
}

const MsgEncryptor::PublicKeyShared InitChannelMessage::publicKey() const
noexcept
{
    return mPublicKey;
}

pair<BytesShared, size_t> InitChannelMessage::serializeToBytes() const
{
    // todo: use serializer

    auto parentBytesAndCount = TransactionMessage::serializeToBytes();

    size_t bytesCount = parentBytesAndCount.second
                        + sizeof(ContractorID)
                        + sizeof(mPublicKey->key);

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
    //----------------------------
    memcpy(
        dataBytesShared.get() + dataBytesOffset + sizeof(ContractorID),
        &mPublicKey->key,
        sizeof(mPublicKey->key));
    //----------------------------
    return make_pair(
        dataBytesShared,
        bytesCount);
}

const bool InitChannelMessage::isAddToConfirmationRequiredMessagesHandler() const
{
    return true;
}