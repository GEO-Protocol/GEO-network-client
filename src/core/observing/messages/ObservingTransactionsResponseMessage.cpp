#include "ObservingTransactionsResponseMessage.h"

ObservingTransactionsResponseMessage::ObservingTransactionsResponseMessage(
    BytesShared buffer) :
    ObservingResponseMessage(
        buffer)
{
    size_t bytesBufferOffset = ObservingResponseMessage::kOffsetToInheritedBytes();

    memcpy(
        &mActualBlockNumber,
        buffer.get() + bytesBufferOffset,
        sizeof(BlockNumber));
    bytesBufferOffset += sizeof(BlockNumber);

    SerializedRecordsCount kRecordsCount;
    memcpy(
        &kRecordsCount,
        buffer.get() + bytesBufferOffset,
        sizeof(SerializedRecordsCount));
    bytesBufferOffset += sizeof(SerializedRecordsCount);

    for (SerializedRecordNumber i = 0; i < kRecordsCount; ++i) {
        auto *state = new (buffer.get() + bytesBufferOffset) ObservingTransaction::SerializedObservingResponseType;
        bytesBufferOffset += sizeof(ObservingTransaction::SerializedObservingResponseType);

        mTransactionsAndResponses.push_back(
            (ObservingTransaction::ObservingResponseType)(*state));
    }
}

BlockNumber ObservingTransactionsResponseMessage::actualBlockNumber() const
{
    return mActualBlockNumber;
}

vector<ObservingTransaction::ObservingResponseType> ObservingTransactionsResponseMessage::transactionsResponses() const
{
    return mTransactionsAndResponses;
}