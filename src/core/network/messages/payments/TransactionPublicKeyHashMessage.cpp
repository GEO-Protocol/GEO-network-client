#include "TransactionPublicKeyHashMessage.h"

TransactionPublicKeyHashMessage::TransactionPublicKeyHashMessage(
    const SerializedEquivalent equivalent,
    const NodeUUID &senderUUID,
    const TransactionUUID &transactionUUID,
    const PaymentNodeID paymentNodeID,
    const uint32_t transactionPublicKeyHash) :

    TransactionMessage(
        equivalent,
        senderUUID,
        transactionUUID),
    mPaymentNodeID(paymentNodeID),
    mTransactionPublicKeyHash(transactionPublicKeyHash),
    mIsReceiptContains(false)
{}

TransactionPublicKeyHashMessage::TransactionPublicKeyHashMessage(
    const SerializedEquivalent equivalent,
    const NodeUUID &senderUUID,
    const TransactionUUID &transactionUUID,
    const PaymentNodeID paymentNodeID,
    const uint32_t transactionPublicKeyHash,
    const TrustLineAmount &amount,
    const KeyNumber publicKeyNumber,
    const lamport::Signature::Shared signature) :

    TransactionMessage(
        equivalent,
        senderUUID,
        transactionUUID),
    mPaymentNodeID(paymentNodeID),
    mTransactionPublicKeyHash(transactionPublicKeyHash),
    mIsReceiptContains(true),
    mAmount(amount),
    mPublicKeyNumber(publicKeyNumber),
    mSignature(signature)
{}

TransactionPublicKeyHashMessage::TransactionPublicKeyHashMessage(
    BytesShared buffer) :
    TransactionMessage(buffer)
{
    auto bytesBufferOffset = TransactionMessage::kOffsetToInheritedBytes();

    memcpy(
        &mPaymentNodeID,
        buffer.get() + bytesBufferOffset,
        sizeof(PaymentNodeID));
    bytesBufferOffset += sizeof(PaymentNodeID);

    memcpy(
        &mTransactionPublicKeyHash,
        buffer.get() + bytesBufferOffset,
        sizeof(uint32_t));
    bytesBufferOffset += sizeof(uint32_t);

    memcpy(
        &mIsReceiptContains,
        buffer.get() + bytesBufferOffset,
        sizeof(byte));

    if (mIsReceiptContains) {
        bytesBufferOffset += sizeof(byte);
        vector<byte> amountBytes(
            buffer.get() + bytesBufferOffset,
            buffer.get() + bytesBufferOffset + kTrustLineAmountBytesCount);
        mAmount = bytesToTrustLineAmount(amountBytes);
        bytesBufferOffset += kTrustLineAmountBytesCount;

        memcpy(
            &mPublicKeyNumber,
            buffer.get() + bytesBufferOffset,
            sizeof(KeyNumber));
        bytesBufferOffset += sizeof(KeyNumber);

        auto signature = make_shared<lamport::Signature>(
            buffer.get() + bytesBufferOffset);
        mSignature = signature;
    }
}

const Message::MessageType TransactionPublicKeyHashMessage::typeID() const
{
    return Message::Payments_TransactionPublicKeyHash;
}

const PaymentNodeID TransactionPublicKeyHashMessage::paymentNodeID() const
{
    return mPaymentNodeID;
}

const uint32_t TransactionPublicKeyHashMessage::transactionPublicKeyHash() const
{
    return mTransactionPublicKeyHash;
}

bool TransactionPublicKeyHashMessage::isReceiptContains() const
{
    return mIsReceiptContains;
}

const TrustLineAmount& TransactionPublicKeyHashMessage::amount() const
{
    return mAmount;
}

const KeyNumber TransactionPublicKeyHashMessage::publicKeyNumber() const
{
    return mPublicKeyNumber;
}

const lamport::Signature::Shared TransactionPublicKeyHashMessage::signature() const
{
    return mSignature;
}

pair<BytesShared, size_t> TransactionPublicKeyHashMessage::serializeToBytes() const
{
    const auto parentBytesAndCount = TransactionMessage::serializeToBytes();

    auto kBufferSize =
            parentBytesAndCount.second
            + sizeof(PaymentNodeID)
            + sizeof(uint32_t)
            + sizeof(byte);
    if (mIsReceiptContains) {
        kBufferSize +=
                kTrustLineAmountBytesCount
                + sizeof(KeyNumber)
                + lamport::Signature::signatureSize();
    }

    BytesShared buffer = tryMalloc(kBufferSize);

    size_t dataBytesOffset = 0;
    memcpy(
        buffer.get(),
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second);
    dataBytesOffset += parentBytesAndCount.second;

    memcpy(
        buffer.get() + dataBytesOffset,
        &mPaymentNodeID,
        sizeof(PaymentNodeID));
    dataBytesOffset += sizeof(PaymentNodeID);

    memcpy(
        buffer.get() + dataBytesOffset,
        &mTransactionPublicKeyHash,
        sizeof(uint32_t));
    dataBytesOffset += sizeof(uint32_t);

    memcpy(
        buffer.get() + dataBytesOffset,
        &mIsReceiptContains,
        sizeof(byte));

    if (mIsReceiptContains) {
        dataBytesOffset += sizeof(byte);
        auto serializedAmount = trustLineAmountToBytes(mAmount);
        memcpy(
            buffer.get() + dataBytesOffset,
            serializedAmount.data(),
            kTrustLineAmountBytesCount);
        dataBytesOffset += kTrustLineAmountBytesCount;

        memcpy(
            buffer.get() + dataBytesOffset,
            &mPublicKeyNumber,
            sizeof(KeyNumber));
        dataBytesOffset += sizeof(KeyNumber);

        memcpy(
            buffer.get() + dataBytesOffset,
            mSignature->data(),
            mSignature->signatureSize());
    }

    return make_pair(
        buffer,
        kBufferSize);
}