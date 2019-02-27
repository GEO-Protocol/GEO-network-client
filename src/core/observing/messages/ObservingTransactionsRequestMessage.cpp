#include "ObservingTransactionsRequestMessage.h"

ObservingTransactionsRequestMessage::ObservingTransactionsRequestMessage(
    vector<pair<TransactionUUID, BlockNumber>> transactions) :
    mTransactions(transactions)
{}

const ObservingMessage::MessageType ObservingTransactionsRequestMessage::typeID() const
{
    return ObservingMessage::Observing_TransactionsRequest;
}

BytesShared ObservingTransactionsRequestMessage::serializeToBytes() const
{
    const auto parentBytesAndCount = ObservingMessage::serializeToBytes();

    BytesShared buffer = tryMalloc(serializedSize());

    size_t dataBytesOffset = 0;
    // Parent message content
    memcpy(
        buffer.get(),
        parentBytesAndCount.get(),
        ObservingMessage::serializedSize());
    dataBytesOffset += ObservingMessage::serializedSize();

    auto transactionsCount = (SerializedRecordsCount)mTransactions.size();
    memcpy(
        buffer.get() + dataBytesOffset,
        &transactionsCount,
        sizeof(SerializedRecordsCount));
    dataBytesOffset += sizeof(SerializedRecordsCount);

    for (const auto &transaction : mTransactions) {
        memcpy(
            buffer.get() + dataBytesOffset,
            &transaction.second,
            sizeof(BlockNumber));
        dataBytesOffset += sizeof(BlockNumber);

        memcpy(
            buffer.get() + dataBytesOffset,
            transaction.first.data,
            TransactionUUID::kBytesSize);
        dataBytesOffset += TransactionUUID::kBytesSize;
    }

    return buffer;
}

size_t ObservingTransactionsRequestMessage::serializedSize() const
{
    return ObservingMessage::serializedSize()
           + sizeof(SerializedRecordsCount)
           + mTransactions.size() * (TransactionUUID::kBytesSize + sizeof(BlockNumber));
}