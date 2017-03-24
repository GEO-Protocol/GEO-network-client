#include "PaymentRecord.h"

namespace db {
    namespace operations_history_storage {

        PaymentRecord::PaymentRecord(
            BytesShared buffer) {

            deserializeFromBytes(
                buffer);
        }

        PaymentRecord::PaymentRecord(
            const uuids::uuid &operationUUID,
            const PaymentRecord::PaymentOperationType operationType,
            const NodeUUID &contractorUUID,
            const TrustLineAmount &amount):

            Record(
                Record::RecordType::PaymentRecordType,
                operationUUID),
            mPaymentOperationType(operationType),
            mContractorUUID(contractorUUID),
            mAmount(amount) {}

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

            auto parentBytesAndCount = Record::serializeToBytes();
            size_t bytesCount = kRecordBytesSize;

            BytesShared bytesBuffer = tryCalloc(
                kRecordBytesSize);
            size_t bytesBufferOffset = 0;

            memcpy(
                bytesBuffer.get(),
                parentBytesAndCount.first.get(),
                parentBytesAndCount.second);
            bytesBufferOffset += parentBytesAndCount.second;

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

        void PaymentRecord::deserializeFromBytes(
            BytesShared buffer) {

            Record::deserializeFromBytes(
                buffer);
            size_t dataBufferOffset = Record::kOffsetToInheritedBytes();

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

    }
}