#include "RequestMessageWithReservations.h"

RequestMessageWithReservations::RequestMessageWithReservations(
    const NodeUUID &senderUUID,
    const TransactionUUID &transactionUUID,
    const vector<pair<PathID, ConstSharedTrustLineAmount>> &finalAmountsConfig) :

    TransactionMessage(
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
    RecordCount *finalAmountsConfigurationCount = new (bytesBufferOffset) RecordCount;
    bytesBufferOffset += sizeof(RecordCount);
    //-----------------------------------------------------
    mFinalAmountsConfiguration.reserve(*finalAmountsConfigurationCount);
    for (RecordNumber idx = 0; idx < *finalAmountsConfigurationCount; idx++) {
        PathID *pathID = new (bytesBufferOffset) PathID;
        bytesBufferOffset += sizeof(PathID);
        //---------------------------------------------------
        vector<byte> bufferTrustLineAmount(
            bytesBufferOffset,
            bytesBufferOffset + kTrustLineAmountBytesCount);
        bytesBufferOffset += kTrustLineAmountBytesCount;
        //---------------------------------------------------
        TrustLineAmount trustLineAmount = bytesToTrustLineAmount(bufferTrustLineAmount);
        mFinalAmountsConfiguration.push_back(
            make_pair(
                *pathID,
                make_shared<const TrustLineAmount>(
                    trustLineAmount)));
    }
}

const vector<pair<Message::PathID, ConstSharedTrustLineAmount>>& RequestMessageWithReservations::finalAmountsConfiguration() const
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
            + sizeof(RecordCount)
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
    RecordCount finalAmountsConfigurationCount = (RecordCount)mFinalAmountsConfiguration.size();
    memcpy(
        bytesBufferOffset,
        &finalAmountsConfigurationCount,
        sizeof(RecordCount));
    bytesBufferOffset += sizeof(RecordCount);
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
