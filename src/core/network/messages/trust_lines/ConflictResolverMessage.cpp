#include "ConflictResolverMessage.h"

ConflictResolverMessage::ConflictResolverMessage(
    const SerializedEquivalent equivalent,
    const NodeUUID &senderUUID,
    const TransactionUUID &transactionUUID,
    AuditRecord::Shared auditRecord,
    vector<ReceiptRecord::Shared> &incomingReceipts,
    vector<ReceiptRecord::Shared> &outgoingReceipts):

    TransactionMessage(
        equivalent,
        senderUUID,
        transactionUUID),
    mAuditRecord(auditRecord),
    mIncomingReceipts(incomingReceipts),
    mOutgoingReceipts(outgoingReceipts)
{}

ConflictResolverMessage::ConflictResolverMessage(
    BytesShared buffer) :
    TransactionMessage(buffer)
{
    auto bytesBufferOffset = TransactionMessage::kOffsetToInheritedBytes();

    mAuditRecord = make_shared<AuditRecord>(
        buffer.get() + bytesBufferOffset);
    bytesBufferOffset += AuditRecord::recordSize();

    auto *incomingReceiptsCount = new (buffer.get() + bytesBufferOffset) SerializedRecordsCount;
    bytesBufferOffset += sizeof(SerializedRecordsCount);
    mIncomingReceipts.reserve(*incomingReceiptsCount);

    for (SerializedRecordNumber idx = 0; idx < *incomingReceiptsCount; idx++) {
        auto incomingReceiptRecord = make_shared<ReceiptRecord>(
            buffer.get() + bytesBufferOffset);
        bytesBufferOffset += ReceiptRecord::recordSize();
        mIncomingReceipts.push_back(
            incomingReceiptRecord);
    }

    auto *outgoingReceiptsCount = new (buffer.get() + bytesBufferOffset) SerializedRecordsCount;
    bytesBufferOffset += sizeof(SerializedRecordsCount);
    mOutgoingReceipts.reserve(*outgoingReceiptsCount);

    for (SerializedRecordNumber idx = 0; idx < *outgoingReceiptsCount; idx++) {
        auto outgoingReceiptRecord = make_shared<ReceiptRecord>(
            buffer.get() + bytesBufferOffset);
        bytesBufferOffset += ReceiptRecord::recordSize();
        mOutgoingReceipts.push_back(
            outgoingReceiptRecord);
    }
}

const Message::MessageType ConflictResolverMessage::typeID() const
{
    return Message::TrustLines_ConflictResolver;
}


AuditRecord::Shared ConflictResolverMessage::auditRecord() const
{
    return mAuditRecord;
}

const vector<ReceiptRecord::Shared> ConflictResolverMessage::incomingReceipts() const
{
    return mIncomingReceipts;
}

const vector<ReceiptRecord::Shared> ConflictResolverMessage::outgoingReceipts() const
{
    return mOutgoingReceipts;
}

const bool ConflictResolverMessage::isAddToConfirmationRequiredMessagesHandler() const
{
    return true;
}

const bool ConflictResolverMessage::isCheckCachedResponse() const
{
    return true;
}

pair<BytesShared, size_t> ConflictResolverMessage::serializeToBytes() const
    throw (bad_alloc)
{
    const auto parentBytesAndCount = TransactionMessage::serializeToBytes();
    auto kBufferSize = parentBytesAndCount.second
                       + AuditRecord::recordSize()
                       + sizeof(SerializedRecordsCount)
                       + mIncomingReceipts.size() * ReceiptRecord::recordSize()
                       + sizeof(SerializedRecordsCount)
                       + mOutgoingReceipts.size() * ReceiptRecord::recordSize();
    BytesShared buffer = tryMalloc(kBufferSize);

    size_t dataBytesOffset = 0;
    // Parent message content
    memcpy(
        buffer.get(),
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second);
    dataBytesOffset += parentBytesAndCount.second;

    auto serializedAuditRecord = mAuditRecord->serializeToBytes();
    memcpy(
        buffer.get() + dataBytesOffset,
        serializedAuditRecord.get(),
        AuditRecord::recordSize());
    dataBytesOffset += AuditRecord::recordSize();

    auto incomingReceipts = (SerializedRecordsCount)mIncomingReceipts.size();
    memcpy(
        buffer.get() + dataBytesOffset,
        &incomingReceipts,
        sizeof(SerializedRecordsCount));
    dataBytesOffset += sizeof(SerializedRecordsCount);

    for (const auto &incomingReceiptRecord : mIncomingReceipts) {
        memcpy(
            buffer.get() + dataBytesOffset,
            incomingReceiptRecord->serializeToBytes().get(),
            ReceiptRecord::recordSize());
        dataBytesOffset += ReceiptRecord::recordSize();
    }

    auto outgoingReceipts = (SerializedRecordsCount)mOutgoingReceipts.size();
    memcpy(
        buffer.get() + dataBytesOffset,
        &outgoingReceipts,
        sizeof(SerializedRecordsCount));
    dataBytesOffset += sizeof(SerializedRecordsCount);

    for (const auto &outgoingReceiptRecord : mOutgoingReceipts) {
        memcpy(
            buffer.get() + dataBytesOffset,
            outgoingReceiptRecord->serializeToBytes().get(),
            ReceiptRecord::recordSize());
        dataBytesOffset += ReceiptRecord::recordSize();
    }

    return make_pair(
        buffer,
        kBufferSize);
}