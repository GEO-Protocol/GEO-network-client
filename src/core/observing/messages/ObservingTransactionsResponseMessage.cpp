#include "ObservingTransactionsResponseMessage.h"

ObservingTransactionsResponseMessage::ObservingTransactionsResponseMessage(
    BytesShared buffer)
{
    size_t bytesBufferOffset = 0;

//    memcpy(
//        &mActualBlockNumber,
//        buffer.get() + bytesBufferOffset,
//        sizeof(BlockNumber));
//    bytesBufferOffset += sizeof(BlockNumber);

    SerializedRecordsCount kRecordsCount;
    memcpy(
        &kRecordsCount,
        buffer.get() + bytesBufferOffset,
        sizeof(SerializedRecordsCount));
    bytesBufferOffset += sizeof(SerializedRecordsCount);

    for (SerializedRecordNumber i = 0; i < kRecordsCount; ++i) {
//        TransactionUUID transactionUUID(buffer.get() + bytesBufferOffset);
//        bytesBufferOffset += TransactionUUID::kBytesSize;
        auto transactionUUID = TransactionUUID::empty();

        auto *state = new (buffer.get() + bytesBufferOffset) ObservingTransaction::SerializedObservingResponseType;
        bytesBufferOffset += sizeof(ObservingTransaction::SerializedObservingResponseType);

        mTransactionsAndResponses.emplace_back(
            transactionUUID,
            (ObservingTransaction::ObservingResponseType)(*state));
    }
}

const ObservingMessage::MessageType ObservingTransactionsResponseMessage::typeID() const
{
    return ObservingMessage::Observing_TransactionsResponse;
}

BlockNumber ObservingTransactionsResponseMessage::actualBlockNumber() const
{
    return mActualBlockNumber;
}

vector<pair<TransactionUUID, ObservingTransaction::ObservingResponseType>> ObservingTransactionsResponseMessage::transactionsAndResponses() const
{
    return mTransactionsAndResponses;
}