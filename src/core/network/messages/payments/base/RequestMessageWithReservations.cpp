#include "RequestMessageWithReservations.h"

RequestMessageWithReservations::RequestMessageWithReservations(
    const SerializedEquivalent equivalent,
    const NodeUUID &senderUUID,
    const TransactionUUID &transactionUUID,
    const vector<pair<PathID, ConstSharedTrustLineAmount>> &finalAmountsConfig) :

    TransactionMessage(
        equivalent,
        senderUUID,
        transactionUUID),

    mFinalAmountsConfiguration(finalAmountsConfig)
{}

RequestMessageWithReservations::RequestMessageWithReservations(
    BytesShared buffer):

    TransactionMessage(buffer)
{
    auto parentMessageOffset = TransactionMessage::kOffsetToInheritedBytes();
    auto bytesBufferOffset = buffer.get() + parentMessageOffset;
    //----------------------------------------------------
    auto *finalAmountsConfigurationCount = new (bytesBufferOffset) SerializedRecordsCount;
    bytesBufferOffset += sizeof(SerializedRecordsCount);
    //-----------------------------------------------------
    mFinalAmountsConfiguration.reserve(*finalAmountsConfigurationCount);
    for (SerializedRecordNumber idx = 0; idx < *finalAmountsConfigurationCount; idx++) {
        auto *pathID = new (bytesBufferOffset) PathID;
        bytesBufferOffset += sizeof(PathID);
        //---------------------------------------------------
        vector<byte> bufferTrustLineAmount(
            bytesBufferOffset,
            bytesBufferOffset + kTrustLineAmountBytesCount);
        bytesBufferOffset += kTrustLineAmountBytesCount;
        //---------------------------------------------------
        TrustLineAmount trustLineAmount = bytesToTrustLineAmount(bufferTrustLineAmount);
        mFinalAmountsConfiguration.emplace_back(
            *pathID,
            make_shared<const TrustLineAmount>(
                trustLineAmount));
    }
}

const vector<pair<PathID, ConstSharedTrustLineAmount>>& RequestMessageWithReservations::finalAmountsConfiguration() const
{
    return mFinalAmountsConfiguration;
}

/*!
 *
 * Throws bad_alloc;
 */
pair<BytesShared, size_t> RequestMessageWithReservations::serializeToBytes() const
    throw (bad_alloc)
{
    auto parentBytesAndCount = TransactionMessage::serializeToBytes();
    size_t bytesCount =
            + parentBytesAndCount.second
            + sizeof(SerializedRecordsCount)
            + mFinalAmountsConfiguration.size() *
              (sizeof(PathID) + kTrustLineAmountBytesCount);

    BytesShared buffer = tryMalloc(bytesCount);

    auto initialOffset = buffer.get();
    memcpy(
        initialOffset,
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second);
    auto bytesBufferOffset = initialOffset + parentBytesAndCount.second;

    //----------------------------------------------------
    auto finalAmountsConfigurationCount = (SerializedRecordsCount)mFinalAmountsConfiguration.size();
    memcpy(
        bytesBufferOffset,
        &finalAmountsConfigurationCount,
        sizeof(SerializedRecordsCount));
    bytesBufferOffset += sizeof(SerializedRecordsCount);
    //----------------------------------------------------
    for (auto const &it : mFinalAmountsConfiguration) {
        memcpy(
            bytesBufferOffset,
            &it.first,
            sizeof(PathID));
        bytesBufferOffset += sizeof(PathID);

        vector<byte> serializedAmount = trustLineAmountToBytes(*it.second.get());
        memcpy(
            bytesBufferOffset,
            serializedAmount.data(),
            kTrustLineAmountBytesCount);
        bytesBufferOffset += kTrustLineAmountBytesCount;
    }
    //----------------------------------------------------
    return make_pair(
        buffer,
        bytesCount);
}

const size_t RequestMessageWithReservations::kOffsetToInheritedBytes() const
    noexcept
{
    auto kOffset =
            TransactionMessage::kOffsetToInheritedBytes()
            + sizeof(SerializedRecordsCount)
            + finalAmountsConfiguration().size() *
              (sizeof(PathID) + kTrustLineAmountBytesCount);

    return kOffset;
}
