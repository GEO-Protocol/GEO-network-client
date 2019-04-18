#include "PaymentRecord.h"

PaymentRecord::PaymentRecord(
    const TransactionUUID &operationUUID,
    const PaymentRecord::PaymentOperationType operationType,
    Contractor::Shared contractor,
    const TrustLineAmount &amount,
    const TrustLineBalance &balanceAfterOperation,
    const string payload):

    Record(
        Record::PaymentRecordType,
        operationUUID,
        contractor),
    mPaymentOperationType(operationType),
    mAmount(amount),
    mBalanceAfterOperation(balanceAfterOperation),
    mCommandUUID(CommandUUID::empty()),
    mPayload(payload)
{}

PaymentRecord::PaymentRecord(
    const TransactionUUID &operationUUID,
    const PaymentRecord::PaymentOperationType operationType,
    Contractor::Shared contractor,
    const TrustLineAmount &amount,
    const TrustLineBalance &balanceAfterOperation,
    const CommandUUID &commandUUID,
    const string payload):

    Record(
        Record::PaymentRecordType,
        operationUUID,
        contractor),
    mPaymentOperationType(operationType),
    mAmount(amount),
    mBalanceAfterOperation(balanceAfterOperation),
    mCommandUUID(commandUUID),
    mPayload(payload)
{}

PaymentRecord::PaymentRecord(
    const TransactionUUID &operationUUID,
    const GEOEpochTimestamp geoEpochTimestamp,
    BytesShared recordBody):
    Record(
        Record::PaymentRecordType,
        operationUUID,
        geoEpochTimestamp)
{
    size_t dataBufferOffset = 0;
    auto *operationType
        = new (recordBody.get() + dataBufferOffset) PaymentRecord::SerializedPaymentOperationType;
    dataBufferOffset += sizeof(
        PaymentRecord::SerializedPaymentOperationType);
    mPaymentOperationType = (PaymentOperationType)*operationType;

    mContractor = make_shared<Contractor>(
        recordBody.get() + dataBufferOffset);
    dataBufferOffset += mContractor->serializedSize();

    vector<byte> amountBytes(
        recordBody.get() + dataBufferOffset,
        recordBody.get() + dataBufferOffset + kTrustLineAmountBytesCount);
    mAmount = bytesToTrustLineAmount(
        amountBytes);
    dataBufferOffset += kTrustLineAmountBytesCount;

    vector<byte> balanceBytes(
        recordBody.get() + dataBufferOffset,
        recordBody.get() + dataBufferOffset + kTrustLineBalanceSerializeBytesCount);
    mBalanceAfterOperation = bytesToTrustLineBalance(
        balanceBytes);
    dataBufferOffset += kTrustLineBalanceSerializeBytesCount;

    byte payloadLength;
    memcpy(
        &payloadLength,
        recordBody.get() + dataBufferOffset,
        sizeof(byte));

    mPayload = "";
    if (payloadLength > 0) {
        dataBufferOffset += sizeof(byte);
        mPayload = string(
            recordBody.get() + dataBufferOffset,
            recordBody.get() + dataBufferOffset + payloadLength);
    }
}

const bool PaymentRecord::isPaymentRecord() const
{
    return true;
}

const PaymentRecord::PaymentOperationType PaymentRecord::paymentOperationType() const
{
    return mPaymentOperationType;
}

const TrustLineAmount& PaymentRecord::amount() const
{
    return mAmount;
}

const TrustLineBalance& PaymentRecord::balanceAfterOperation() const
{
    return mBalanceAfterOperation;
}

const CommandUUID& PaymentRecord::commandUUID() const
{
    return mCommandUUID;
}

const string PaymentRecord::payload() const
{
    return mPayload;
}

pair<BytesShared, size_t> PaymentRecord::serializedHistoryRecordBody() const
{
    size_t recordBodySize = sizeof(SerializedPaymentOperationType)
                            + mContractor->serializedSize()
                            + kTrustLineAmountBytesCount
                            + kTrustLineBalanceSerializeBytesCount
                            + sizeof(byte)
                            + mPayload.length();

    BytesShared bytesBuffer = tryCalloc(
        recordBodySize);
    size_t bytesBufferOffset = 0;

    memcpy(
        bytesBuffer.get() + bytesBufferOffset,
        &mPaymentOperationType,
        sizeof(SerializedPaymentOperationType));
    bytesBufferOffset += sizeof(SerializedPaymentOperationType);

    auto contractorSerializedData = mContractor->serializeToBytes();
    memcpy(
        bytesBuffer.get() + bytesBufferOffset,
        contractorSerializedData.get(),
        mContractor->serializedSize());
    bytesBufferOffset += mContractor->serializedSize();

    auto trustAmountBytes = trustLineAmountToBytes(mAmount);
    memcpy(
        bytesBuffer.get() + bytesBufferOffset,
        trustAmountBytes.data(),
        kTrustLineAmountBytesCount);
    bytesBufferOffset += kTrustLineAmountBytesCount;

    auto trustBalanceBytes = trustLineBalanceToBytes(mBalanceAfterOperation);
    memcpy(
        bytesBuffer.get() + bytesBufferOffset,
        trustBalanceBytes.data(),
        kTrustLineBalanceSerializeBytesCount);
    bytesBufferOffset += kTrustLineBalanceSerializeBytesCount;

    auto payloadLength = (byte)mPayload.length();
    memcpy(
        bytesBuffer.get() + bytesBufferOffset,
        &payloadLength,
        sizeof(byte));

    if (payloadLength > 0) {
        bytesBufferOffset += sizeof(byte);
        memcpy(
            bytesBuffer.get() + bytesBufferOffset,
            mPayload.c_str(),
            payloadLength);
    }

    return make_pair(
        bytesBuffer,
        recordBodySize);
}