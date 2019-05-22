#include "UpdateChannelAddressesMessage.h"

UpdateChannelAddressesMessage::UpdateChannelAddressesMessage(
    ContractorID idOnSenderSide,
    const TransactionUUID &transactionUUID,
    vector<BaseAddress::Shared> newSenderAddresses,
    Contractor::Shared contractor) :
    TransactionMessage(
        0,
        idOnSenderSide,
        transactionUUID),
    mNewSenderAddresses(newSenderAddresses)
{
    encrypt(contractor);
}

UpdateChannelAddressesMessage::UpdateChannelAddressesMessage(
    BytesShared buffer):
    TransactionMessage(buffer)
{
    size_t bytesBufferOffset = TransactionMessage::kOffsetToInheritedBytes();

    byte senderAddressesCnt;
    memcpy(
        &senderAddressesCnt,
        buffer.get() + bytesBufferOffset,
        sizeof(byte));
    bytesBufferOffset += sizeof(byte);
    mNewSenderAddresses.reserve(
        senderAddressesCnt);

    for (int idx = 0; idx < senderAddressesCnt; idx++) {
        auto senderAddress = deserializeAddress(
            buffer.get() + bytesBufferOffset);
        mNewSenderAddresses.push_back(senderAddress);
        bytesBufferOffset += senderAddress->serializedSize();
    }
}

const Message::MessageType UpdateChannelAddressesMessage::typeID() const
{
    return Message::Channel_UpdateAddresses;
}

vector<BaseAddress::Shared> UpdateChannelAddressesMessage::newSenderAddresses() const
{
    return mNewSenderAddresses;
}

pair<BytesShared, size_t> UpdateChannelAddressesMessage::serializeToBytes() const
{
    auto parentBytesAndCount = TransactionMessage::serializeToBytes();

    size_t bytesCount = parentBytesAndCount.second + sizeof(byte);
    for (const auto &address : mNewSenderAddresses) {
        bytesCount += address->serializedSize();
    }

    BytesShared dataBytesShared = tryCalloc(bytesCount);
    size_t dataBytesOffset = 0;
    //----------------------------------------------------
    memcpy(
        dataBytesShared.get(),
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second);
    dataBytesOffset += parentBytesAndCount.second;
    //----------------------------
    auto addressesCnt = (byte)mNewSenderAddresses.size();
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        &addressesCnt,
        sizeof(byte));
    dataBytesOffset += sizeof(byte);
    //----------------------------
    for (const auto &address : mNewSenderAddresses) {
        auto serializedAddress = address->serializeToBytes();
        memcpy(
            dataBytesShared.get() + dataBytesOffset,
            serializedAddress.get(),
            address->serializedSize());
        dataBytesOffset += address->serializedSize();
    }
    //----------------------------
    return make_pair(
        dataBytesShared,
        bytesCount);
}

const bool UpdateChannelAddressesMessage::isAddToConfirmationRequiredMessagesHandler() const
{
    return true;
}