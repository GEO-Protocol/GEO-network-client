#include "TransactionPublicKeyHashMessage.h"

TransactionPublicKeyHashMessage::TransactionPublicKeyHashMessage(
    const SerializedEquivalent equivalent,
    const NodeUUID &senderUUID,
    const TransactionUUID &transactionUUID,
    const PaymentNodeID paymentNodeID,
    const lamport::KeyHash::Shared transactionPublicKeyHash) :

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
    const lamport::KeyHash::Shared transactionPublicKeyHash,
    const KeyNumber publicKeyNumber,
    const lamport::Signature::Shared signature) :

    TransactionMessage(
        equivalent,
        senderUUID,
        transactionUUID),
    mPaymentNodeID(paymentNodeID),
    mTransactionPublicKeyHash(transactionPublicKeyHash),
    mIsReceiptContains(true),
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

    mTransactionPublicKeyHash = make_shared<lamport::KeyHash>(
        buffer.get() + bytesBufferOffset);
    bytesBufferOffset += lamport::KeyHash::kBytesSize;

    memcpy(
        &mIsReceiptContains,
        buffer.get() + bytesBufferOffset,
        sizeof(byte));

    if (mIsReceiptContains) {
        bytesBufferOffset += sizeof(byte);
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

const lamport::KeyHash::Shared TransactionPublicKeyHashMessage::transactionPublicKeyHash() const
{
    return mTransactionPublicKeyHash;
}

bool TransactionPublicKeyHashMessage::isReceiptContains() const
{
    return mIsReceiptContains;
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
            + lamport::KeyHash::kBytesSize
            + sizeof(byte);
    if (mIsReceiptContains) {
        kBufferSize += (sizeof(KeyNumber)
                + lamport::Signature::signatureSize());
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
        mTransactionPublicKeyHash->data(),
        lamport::KeyHash::kBytesSize);
    dataBytesOffset += lamport::KeyHash::kBytesSize;

    memcpy(
        buffer.get() + dataBytesOffset,
        &mIsReceiptContains,
        sizeof(byte));

    if (mIsReceiptContains) {
        dataBytesOffset += sizeof(byte);
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