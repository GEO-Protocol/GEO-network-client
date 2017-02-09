#include "ReceiverInitPaymentMessage.h"


ReceiverInitPaymentMessage::ReceiverInitPaymentMessage(
    const TrustLineAmount &totalPaymentAmount) :

    mTotalPaymentAmount(totalPaymentAmount){
}

const Message::MessageType ReceiverInitPaymentMessage::typeID() const {
    return Message::ReceiverInitPaymentMessageType;
}

/*!
 *
 * Throws bad_alloc;
 */
pair<BytesShared, size_t> ReceiverInitPaymentMessage::serializeToBytes() const {

    auto serializedPaymentAmount =
        trustLineAmountToBytes(
            mTotalPaymentAmount); // todo: serialize only non-zero

    auto parentBytesAndCount = TransactionMessage::serializeToBytes();
    size_t bytesCount =
        + parentBytesAndCount.second
        + kTrustLineAmountBytesCount;

    BytesShared buffer =
        tryMalloc(
            bytesCount);

    auto initialOffset = buffer.get();
    memcpy(
        initialOffset,
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second);

    auto amountOffset = initialOffset + parentBytesAndCount.second;
    memcpy(
        amountOffset,
        serializedPaymentAmount.data(),
        kTrustLineAmountBytesCount);

    return make_pair(
        buffer,
        bytesCount);
}

void ReceiverInitPaymentMessage::deserializeFromBytes(BytesShared buffer) {

    TransactionMessage::deserializeFromBytes(buffer);

    auto parentMessageOffset = TransactionMessage::kOffsetToInheritedBytes();
    auto amountOffset = buffer.get() + parentMessageOffset;
    auto amountEndOffset = amountOffset + kTrustLineBalanceBytesCount; // todo: deserialize only non-zero
    vector<byte> amountBytes(
        amountOffset,
        amountEndOffset);

    mTotalPaymentAmount = bytesToTrustLineAmount(amountBytes);
}
