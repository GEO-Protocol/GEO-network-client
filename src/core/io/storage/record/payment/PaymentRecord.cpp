#include "PaymentRecord.h"

PaymentRecord::PaymentRecord(
    const SerializedEquivalent equivalent,
    const TransactionUUID &operationUUID,
    const PaymentRecord::PaymentOperationType operationType,
    Contractor::Shared contractor,
    const TrustLineAmount &amount,
    const TrustLineBalance &balanceAfterOperation,
    vector<pair<ContractorID, TrustLineAmount>> &outgoingTransfers,
    vector<pair<ContractorID, TrustLineAmount>> &incomingTransfers,
    const string payload):

    Record(
        Record::PaymentRecordType,
        operationUUID,
        contractor),
    mEquivalent(equivalent),
    mPaymentOperationType(operationType),
    mAmount(amount),
    mBalanceAfterOperation(balanceAfterOperation),
    mOutgoingTransfers(outgoingTransfers),
    mIncomingTransfers(incomingTransfers),
    mCommandUUID(CommandUUID::empty()),
    mPayload(payload)
{}

PaymentRecord::PaymentRecord(
    const SerializedEquivalent equivalent,
    const TransactionUUID &operationUUID,
    const PaymentRecord::PaymentOperationType operationType,
    Contractor::Shared contractor,
    const TrustLineAmount &amount,
    const TrustLineBalance &balanceAfterOperation,
    vector<pair<ContractorID, TrustLineAmount>> &outgoingTransfers,
    vector<pair<ContractorID, TrustLineAmount>> &incomingTransfers,
    const CommandUUID &commandUUID,
    const string payload):

    Record(
        Record::PaymentRecordType,
        operationUUID,
        contractor),
    mEquivalent(equivalent),
    mPaymentOperationType(operationType),
    mAmount(amount),
    mBalanceAfterOperation(balanceAfterOperation),
    mOutgoingTransfers(outgoingTransfers),
    mIncomingTransfers(incomingTransfers),
    mCommandUUID(commandUUID),
    mPayload(payload)
{}

PaymentRecord::PaymentRecord(
    const SerializedEquivalent equivalent,
    const TransactionUUID &operationUUID,
    const GEOEpochTimestamp geoEpochTimestamp,
    BytesShared recordBody):
    Record(
        Record::PaymentRecordType,
        operationUUID,
        geoEpochTimestamp),
    mEquivalent(equivalent)
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

    uint16_t outgoingTransfersCount;
    memcpy(
        &outgoingTransfersCount,
        recordBody.get() + dataBufferOffset,
        sizeof(uint16_t));
    dataBufferOffset += sizeof(uint16_t);

    mOutgoingTransfers.reserve(outgoingTransfersCount);
    for (uint16_t idx = 0; idx < outgoingTransfersCount; idx++) {
        ContractorID contractorID;
        memcpy(
            &contractorID,
            recordBody.get() + dataBufferOffset,
            sizeof(ContractorID));
        dataBufferOffset += sizeof(ContractorID);

        vector<byte> amountTransferBytes(
            recordBody.get() + dataBufferOffset,
            recordBody.get() + dataBufferOffset + kTrustLineAmountBytesCount);
        dataBufferOffset += kTrustLineAmountBytesCount;

        mOutgoingTransfers.emplace_back(
            contractorID,
            bytesToTrustLineAmount(
                amountTransferBytes));
    }

    uint16_t incomingTransfersCount;
    memcpy(
        &incomingTransfersCount,
        recordBody.get() + dataBufferOffset,
        sizeof(uint16_t));
    dataBufferOffset += sizeof(uint16_t);

    mIncomingTransfers.reserve(incomingTransfersCount);
    for (uint16_t idx = 0; idx < incomingTransfersCount; idx++) {
        ContractorID contractorID;
        memcpy(
            &contractorID,
            recordBody.get() + dataBufferOffset,
            sizeof(ContractorID));
        dataBufferOffset += sizeof(ContractorID);

        vector<byte> amountTransferBytes(
            recordBody.get() + dataBufferOffset,
            recordBody.get() + dataBufferOffset + kTrustLineAmountBytesCount);
        dataBufferOffset += kTrustLineAmountBytesCount;

        mIncomingTransfers.emplace_back(
            contractorID,
            bytesToTrustLineAmount(
                amountTransferBytes));
    }

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

const SerializedEquivalent PaymentRecord::equivalent() const
{
    return mEquivalent;
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
                            + sizeof(uint16_t)
                            + (sizeof(ContractorID) + kTrustLineAmountBytesCount) * mOutgoingTransfers.size()
                            + sizeof(uint16_t)
                            + (sizeof(ContractorID) + kTrustLineAmountBytesCount) * mIncomingTransfers.size()
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

    auto outgoingTransfersCount = (uint16_t)mOutgoingTransfers.size();
    memcpy(
        bytesBuffer.get() + bytesBufferOffset,
        &outgoingTransfersCount,
        sizeof(uint16_t));
    bytesBufferOffset += sizeof(uint16_t);

    for (const auto &outgoingTransfer : mOutgoingTransfers) {
        auto contractorID = outgoingTransfer.first;
        memcpy(
            bytesBuffer.get() + bytesBufferOffset,
            &contractorID,
            sizeof(ContractorID));
        bytesBufferOffset += sizeof(ContractorID);

        auto transferAmountBytes = trustLineAmountToBytes(outgoingTransfer.second);
        memcpy(
            bytesBuffer.get() + bytesBufferOffset,
            transferAmountBytes.data(),
            kTrustLineAmountBytesCount);
        bytesBufferOffset += kTrustLineAmountBytesCount;
    }

    auto incomingTransfersCount = (uint16_t)mIncomingTransfers.size();
    memcpy(
        bytesBuffer.get() + bytesBufferOffset,
        &incomingTransfersCount,
        sizeof(uint16_t));
    bytesBufferOffset += sizeof(uint16_t);

    for (const auto &incomingTransfer : mIncomingTransfers) {
        auto contractorID = incomingTransfer.first;
        memcpy(
            bytesBuffer.get() + bytesBufferOffset,
            &contractorID,
            sizeof(ContractorID));
        bytesBufferOffset += sizeof(ContractorID);

        auto transferAmountBytes = trustLineAmountToBytes(incomingTransfer.second);
        memcpy(
            bytesBuffer.get() + bytesBufferOffset,
            transferAmountBytes.data(),
            kTrustLineAmountBytesCount);
        bytesBufferOffset += kTrustLineAmountBytesCount;
    }

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