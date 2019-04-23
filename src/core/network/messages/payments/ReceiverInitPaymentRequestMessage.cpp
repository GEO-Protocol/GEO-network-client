#include "ReceiverInitPaymentRequestMessage.h"

ReceiverInitPaymentRequestMessage::ReceiverInitPaymentRequestMessage(
    const SerializedEquivalent equivalent,
    vector<BaseAddress::Shared> &senderAddresses,
    const TransactionUUID &transactionUUID,
    const TrustLineAmount &amount,
    const string payload) :
    RequestMessage(
        equivalent,
        senderAddresses,
        transactionUUID,
        0,
        amount),
    mPayload(payload)
{}

ReceiverInitPaymentRequestMessage::ReceiverInitPaymentRequestMessage(
    BytesShared buffer) :
    RequestMessage(
        buffer)
{
    auto bytesBufferOffset = RequestMessage::kOffsetToInheritedBytes();
    auto *payloadLength = new (buffer.get() + bytesBufferOffset) PayloadLength;
    if (*payloadLength > 0) {
        bytesBufferOffset += sizeof(PayloadLength);
        mPayload = string(
            buffer.get() + bytesBufferOffset,
            buffer.get() + bytesBufferOffset + *payloadLength);
    } else {
        mPayload = "";
    }
}

const Message::MessageType ReceiverInitPaymentRequestMessage::typeID() const
{
    return Message::Payments_ReceiverInitPaymentRequest;
}

const string ReceiverInitPaymentRequestMessage::payload() const
{
    return mPayload;
}

pair<BytesShared, size_t> ReceiverInitPaymentRequestMessage::serializeToBytes() const
{
    auto parentBytesAndCount = RequestMessage::serializeToBytes();
    size_t bytesCount =
            + parentBytesAndCount.second
            + sizeof(PayloadLength)
            + mPayload.length();

    BytesShared buffer = tryMalloc(bytesCount);
    size_t bytesBufferOffset = 0;
    memcpy(
        buffer.get() + bytesBufferOffset,
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second);
    bytesBufferOffset += parentBytesAndCount.second;

    auto payloadLength = (PayloadLength)mPayload.length();
    memcpy(
        buffer.get() + bytesBufferOffset,
        &payloadLength,
        sizeof(PayloadLength));

    if (payloadLength > 0) {
        bytesBufferOffset += sizeof(PayloadLength);
        memcpy(
            buffer.get() + bytesBufferOffset,
            mPayload.c_str(),
            payloadLength);
    }

    return make_pair(
        buffer,
        bytesCount);
}
