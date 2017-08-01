#include "FinalAmountsConfigurationMessage.h"

FinalAmountsConfigurationMessage::FinalAmountsConfigurationMessage(
    const NodeUUID &senderUUID,
    const TransactionUUID &transactionUUID,
    const vector<pair<PathUUID, ConstSharedTrustLineAmount>> &finalAmountsConfig) :

    TransactionMessage(
        senderUUID,
        transactionUUID),

    mFinalAmountsConfiguration(finalAmountsConfig)
{}

FinalAmountsConfigurationMessage::FinalAmountsConfigurationMessage(
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
        PathUUID *pathUUID = new (bytesBufferOffset) PathUUID;
        bytesBufferOffset += sizeof(PathUUID);
        //---------------------------------------------------
        vector<byte> bufferTrustLineAmount(
            bytesBufferOffset,
            bytesBufferOffset + kTrustLineAmountBytesCount);
        bytesBufferOffset += kTrustLineAmountBytesCount;
        //---------------------------------------------------
        TrustLineAmount trustLineAmount = bytesToTrustLineAmount(bufferTrustLineAmount);
        mFinalAmountsConfiguration.push_back(
            make_pair(
                *pathUUID,
                make_shared<const TrustLineAmount>(
                    trustLineAmount)));
    }
}

const vector<pair<Message::PathUUID, ConstSharedTrustLineAmount>>& FinalAmountsConfigurationMessage::finalAmountsConfiguration() const
{
    return mFinalAmountsConfiguration;
}

/*!
 *
 * Throws bad_alloc;
 */
pair<BytesShared, size_t> FinalAmountsConfigurationMessage::serializeToBytes() const
    throw (bad_alloc)
{
    auto parentBytesAndCount = TransactionMessage::serializeToBytes();
    size_t bytesCount =
            + parentBytesAndCount.second
            + sizeof(RecordCount)
            + mFinalAmountsConfiguration.size() *
              (sizeof(PathUUID) + kTrustLineAmountBytesCount);

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
            sizeof(PathUUID));
        bytesBufferOffset += sizeof(PathUUID);

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

const Message::MessageType FinalAmountsConfigurationMessage::typeID() const
{
    return Message::Payments_FinalAmountsConfiguration;
}

const size_t FinalAmountsConfigurationMessage::kOffsetToInheritedBytes() const
    noexcept
{
    static const size_t offset =
            TransactionMessage::kOffsetToInheritedBytes()
            + sizeof(RecordCount)
            + mFinalAmountsConfiguration.size() *
              (sizeof(PathUUID) + kTrustLineAmountBytesCount);

    return offset;
}
