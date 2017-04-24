#include "PaymentRecord.h"

PaymentRecord::PaymentRecord(
    const uuids::uuid &operationUUID,
    BytesShared buffer):

    Record(
        Record::RecordType::PaymentRecordType,
        operationUUID)
{
    size_t dataBufferOffset = 0;

    SerializedPaymentOperationType *operationType = new (buffer.get() + dataBufferOffset) SerializedPaymentOperationType;
    mPaymentOperationType = (PaymentRecord::PaymentOperationType) *operationType;
    dataBufferOffset += sizeof(
        SerializedPaymentOperationType);

    memcpy(
        mContractorUUID.data,
        buffer.get() + dataBufferOffset,
        NodeUUID::kBytesSize);
    dataBufferOffset += NodeUUID::kBytesSize;

    vector<byte> amountBytes(
        buffer.get() + dataBufferOffset,
        buffer.get() + dataBufferOffset + kTrustLineAmountBytesCount);
    mAmount = bytesToTrustLineAmount(
        amountBytes);
    dataBufferOffset += kTrustLineAmountBytesCount;

    vector<byte> balanceBytes(
        buffer.get() + dataBufferOffset,
        buffer.get() + dataBufferOffset + kTrustLineBalanceBytesCount);
    mBalanceAfterOperation = bytesToTrustLineBalance(
        balanceBytes);
}

PaymentRecord::PaymentRecord(
    const uuids::uuid &operationUUID,
    const PaymentRecord::PaymentOperationType operationType,
    const NodeUUID &contractorUUID,
    const TrustLineAmount &amount,
    const TrustLineBalance &balanceAfterOperation):

    Record(
        Record::RecordType::PaymentRecordType,
        operationUUID),
    mPaymentOperationType(operationType),
    mContractorUUID(contractorUUID),
    mAmount(amount),
    mBalanceAfterOperation(balanceAfterOperation) {}

const bool PaymentRecord::isPaymentRecord() const {

    return true;
}

const PaymentRecord::PaymentOperationType PaymentRecord::paymentOperationType() const {

    return mPaymentOperationType;
}

const NodeUUID PaymentRecord::contractorUUID() const {

    return mContractorUUID;
}

const TrustLineAmount PaymentRecord::amount() const {

    return mAmount;
}

const TrustLineBalance PaymentRecord::balanceAfterOperation() const {

    return mBalanceAfterOperation;
}

pair<BytesShared, size_t> PaymentRecord::serializeToBytes() {

    size_t bytesCount = recordSize();

    BytesShared bytesBuffer = tryCalloc(
        bytesCount);
    size_t bytesBufferOffset = 0;

    SerializedPaymentOperationType operationType = (SerializedPaymentOperationType) mPaymentOperationType;
    memcpy(
        bytesBuffer.get() + bytesBufferOffset,
        &operationType,
        sizeof(SerializedPaymentOperationType));
    bytesBufferOffset += sizeof(
        SerializedPaymentOperationType);

    memcpy(
        bytesBuffer.get() + bytesBufferOffset,
        mContractorUUID.data,
        NodeUUID::kBytesSize);
    bytesBufferOffset += NodeUUID::kBytesSize;

    auto trustAmountBytes = trustLineAmountToBytes(
        mAmount);
    memcpy(
        bytesBuffer.get() + bytesBufferOffset,
        trustAmountBytes.data(),
        kTrustLineAmountBytesCount);
    bytesBufferOffset += kTrustLineAmountBytesCount;

    auto trustBalanceBytes = trustLineBalanceToBytes(
        mBalanceAfterOperation);
    memcpy(
        bytesBuffer.get() + bytesBufferOffset,
        trustBalanceBytes.data(),
        kTrustLineBalanceBytesCount);

    return make_pair(
        bytesBuffer,
        bytesCount);
}

size_t PaymentRecord::recordSize() {

    return sizeof(SerializedPaymentOperationType)
           + NodeUUID::kBytesSize
           + kTrustLineAmountBytesCount
           + kTrustLineBalanceBytesCount;
}