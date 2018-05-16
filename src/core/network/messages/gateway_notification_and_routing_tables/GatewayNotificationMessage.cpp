#include "GatewayNotificationMessage.h"

GatewayNotificationMessage::GatewayNotificationMessage(
    const NodeUUID& senderUUID,
    const TransactionUUID& transactionUUID,
    const vector<SerializedEquivalent> gatewayEquivalents) :

    TransactionMessage(
        0,
        senderUUID,
        transactionUUID),
    mGatewayEquivalents(gatewayEquivalents)
{}

GatewayNotificationMessage::GatewayNotificationMessage(
    BytesShared buffer):

    TransactionMessage(buffer)
{
    size_t bytesBufferOffset = TransactionMessage::kOffsetToInheritedBytes();
    //----------------------------------------------------
    SerializedRecordsCount *equivalentGatewaysCount = new (buffer.get() + bytesBufferOffset) SerializedRecordsCount;
    bytesBufferOffset += sizeof(SerializedRecordsCount);
    //-----------------------------------------------------
    mGatewayEquivalents.reserve(*equivalentGatewaysCount);
    for (SerializedRecordNumber idx = 0; idx < *equivalentGatewaysCount; idx++) {
        SerializedEquivalent gatewayEquivalent;
        memcpy(
            &gatewayEquivalent,
            buffer.get() + Message::kOffsetToInheritedBytes(),
            sizeof(SerializedEquivalent));
        bytesBufferOffset += sizeof(SerializedEquivalent);
        //---------------------------------------------------
        mGatewayEquivalents.push_back(
            gatewayEquivalent);
    }
}

const vector<SerializedEquivalent> GatewayNotificationMessage::gatewayEquivalents() const
{
    return mGatewayEquivalents;
}

pair<BytesShared, size_t> GatewayNotificationMessage::serializeToBytes() const
{
    auto parentBytesAndCount = TransactionMessage::serializeToBytes();

    size_t bytesCount =
        parentBytesAndCount.second
        + sizeof(SerializedRecordsCount)
        + mGatewayEquivalents.size() * sizeof(SerializedEquivalent);

    BytesShared dataBytesShared = tryMalloc(bytesCount);
    size_t dataBytesOffset = 0;
    //----------------------------------------------------
    memcpy(
        dataBytesShared.get(),
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second);
    dataBytesOffset += parentBytesAndCount.second;
    //----------------------------------------------------
    SerializedRecordsCount equivalentGatewaysCount = (SerializedRecordsCount)mGatewayEquivalents.size();
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        &equivalentGatewaysCount,
        sizeof(SerializedRecordsCount));
    dataBytesOffset += sizeof(SerializedRecordsCount);
    //----------------------------------------------------
    for (auto const &gatewayEquivalent : mGatewayEquivalents) {
        memcpy(
            dataBytesShared.get() + dataBytesOffset,
            &gatewayEquivalent,
            sizeof(SerializedEquivalent));
        dataBytesOffset += sizeof(SerializedEquivalent);
    }
    //----------------------------------------------------
    return make_pair(
        dataBytesShared,
        bytesCount);
}

const Message::MessageType GatewayNotificationMessage::typeID() const
{
    return Message::GatewayNotification;
}

const bool GatewayNotificationMessage::isAddToConfirmationRequiredMessagesHandler() const
{
    return true;
}