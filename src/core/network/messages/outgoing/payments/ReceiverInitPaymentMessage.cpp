#include "ReceiverInitPaymentMessage.h"


ReceiverInitPaymentMessage::ReceiverInitPaymentMessage(
    const NodeUUID &senderUUID,
    const TransactionUUID &transactionUUID,
    const TrustLineAmount &totalPaymentAmount) :

    // TODO: add "const" to constructor of the TransactionMessage;
    // TODO: remove const_cast;
    TransactionMessage(
        const_cast<NodeUUID&>(senderUUID),
        const_cast<TransactionUUID&>(transactionUUID)),
    mTotalPaymentAmount(totalPaymentAmount){
}

ReceiverInitPaymentMessage::ReceiverInitPaymentMessage(
    BytesShared buffer) {

    deserializeFromBytes(buffer);
}

const Message::MessageType ReceiverInitPaymentMessage::typeID() const {
    return Message::Payments_ReceiverInitPayment;
}

TrustLineAmount&ReceiverInitPaymentMessage::amount() const {
    return mTotalPaymentAmount;
}

/*!
 *
 * Throws bad_alloc;
 */
pair<BytesShared, size_t> ReceiverInitPaymentMessage::serializeToBytes() {

    auto serializedPaymentAmount =
        trustLineAmountToBytes(
            mTotalPaymentAmount); // TODO: serialize only non-zero

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
    auto amountEndOffset = amountOffset + kTrustLineBalanceBytesCount; // TODO: deserialize only non-zero
    vector<byte> amountBytes(
        amountOffset,
        amountEndOffset);

    mTotalPaymentAmount = bytesToTrustLineAmount(amountBytes);
}
