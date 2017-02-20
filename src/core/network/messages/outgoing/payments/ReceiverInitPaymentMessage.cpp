#include "ReceiverInitPaymentMessage.h"

ReceiverInitPaymentMessage::ReceiverInitPaymentMessage(
    const TrustLineAmount &totalPaymentAmount) :

    mTotalPaymentAmount(totalPaymentAmount){
}

ReceiverInitPaymentMessage::ReceiverInitPaymentMessage(
    BytesShared buffer) {

    deserializeFromBytes(buffer);
}

const Message::MessageType ReceiverInitPaymentMessage::typeID() const {

    return Message::ReceiverInitPaymentMessageType;
}

pair<BytesShared, size_t> ReceiverInitPaymentMessage::serializeToBytes() {

    auto parentBytesAndCount = TransactionMessage::serializeToBytes();

    size_t bytesCount = parentBytesAndCount.second
                        + kTrustLineAmountBytesCount;

    BytesShared dataBytesShared = tryMalloc(bytesCount);
    size_t dataBytesOffset = 0;
    //----------------------------------------------------
    memcpy(
        dataBytesShared.get(),
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second
    );
    dataBytesOffset += parentBytesAndCount.second;
    //----------------------------------------------------
    // todo: serialize only non-zero
    auto serializedPaymentAmount = trustLineAmountToBytes(mTotalPaymentAmount);
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        serializedPaymentAmount.data(),
        kTrustLineAmountBytesCount
    );
    //----------------------------------------------------
    return make_pair(
        dataBytesShared,
        bytesCount
    );
}

void ReceiverInitPaymentMessage::deserializeFromBytes(
    BytesShared buffer) {

    TransactionMessage::deserializeFromBytes(buffer);
    size_t bytesBufferOffset = TransactionMessage::kOffsetToInheritedBytes();
    //----------------------------------------------------
    // todo: deserialize only non-zero
    vector<byte> amountBytes(
        buffer.get() + bytesBufferOffset,
        buffer.get() + bytesBufferOffset + kTrustLineBalanceBytesCount
    );

    mTotalPaymentAmount = bytesToTrustLineAmount(amountBytes);
}
