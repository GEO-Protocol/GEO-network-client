#include "PaymentAdditionalRecord.h"

PaymentAdditionalRecord::PaymentAdditionalRecord(
    const TransactionUUID &operationUUID,
    const PaymentAdditionalOperationType operationType,
    const TrustLineAmount &amount,
    vector<pair<ContractorID, TrustLineAmount>> &outgoingTransfers,
    vector<pair<ContractorID, TrustLineAmount>> &incomingTransfers):

    Record(
        Record::PaymentAdditionalRecordType,
        operationUUID,
        nullptr),
    mPaymentOperationType(operationType),
    mAmount(amount),
    mOutgoingTransfers(outgoingTransfers),
    mIncomingTransfers(incomingTransfers)
{}

PaymentAdditionalRecord::PaymentAdditionalRecord(
    const TransactionUUID &operationUUID,
    const GEOEpochTimestamp geoEpochTimestamp,
    BytesShared recordBody) :
    Record(
        Record::PaymentAdditionalRecordType,
        operationUUID,
        geoEpochTimestamp)
{
    size_t dataBufferOffset = 0;
    auto operationType
        = new (recordBody.get() + dataBufferOffset) PaymentAdditionalRecord::SerializedPaymentOperationType;
    dataBufferOffset += sizeof(
        PaymentAdditionalRecord::SerializedPaymentOperationType);
    mPaymentOperationType = (PaymentAdditionalOperationType)*operationType;

    vector<byte> amountBytes(
        recordBody.get() + dataBufferOffset,
        recordBody.get() + dataBufferOffset + kTrustLineAmountBytesCount);
    mAmount = bytesToTrustLineAmount(
        amountBytes);
    dataBufferOffset += kTrustLineAmountBytesCount;

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
}

const PaymentAdditionalRecord::PaymentAdditionalOperationType PaymentAdditionalRecord::operationType() const
{
    return mPaymentOperationType;
}

const TrustLineAmount& PaymentAdditionalRecord::amount() const
{
    return mAmount;
}

pair<BytesShared, size_t> PaymentAdditionalRecord::serializedHistoryRecordBody() const
{
    size_t recordBodySize = sizeof(SerializedPaymentOperationType)
                            + kTrustLineAmountBytesCount
                            + sizeof(uint16_t)
                            + (sizeof(ContractorID) + kTrustLineAmountBytesCount) * mOutgoingTransfers.size()
                            + sizeof(uint16_t)
                            + (sizeof(ContractorID) + kTrustLineAmountBytesCount) * mIncomingTransfers.size();

    BytesShared bytesBuffer = tryCalloc(
        recordBodySize);
    size_t bytesBufferOffset = 0;

    memcpy(
        bytesBuffer.get() + bytesBufferOffset,
        &mPaymentOperationType,
        sizeof(SerializedPaymentOperationType));
    bytesBufferOffset += sizeof(SerializedPaymentOperationType);

    auto trustAmountBytes = trustLineAmountToBytes(mAmount);
    memcpy(
        bytesBuffer.get() + bytesBufferOffset,
        trustAmountBytes.data(),
        kTrustLineAmountBytesCount);
    bytesBufferOffset += kTrustLineAmountBytesCount;

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

    return make_pair(
        bytesBuffer,
        recordBodySize);
}