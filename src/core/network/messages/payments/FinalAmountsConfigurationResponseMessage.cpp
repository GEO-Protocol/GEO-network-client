#include "FinalAmountsConfigurationResponseMessage.h"

FinalAmountsConfigurationResponseMessage::FinalAmountsConfigurationResponseMessage(
    const SerializedEquivalent equivalent,
    const NodeUUID& senderUUID,
    const TransactionUUID& transactionUUID,
    const OperationState state) :

    TransactionMessage(
        equivalent,
        senderUUID,
        transactionUUID),
    mState(state)
{}

FinalAmountsConfigurationResponseMessage::FinalAmountsConfigurationResponseMessage(
    const SerializedEquivalent equivalent,
    const NodeUUID& senderUUID,
    const TransactionUUID& transactionUUID,
    const OperationState state,
    const CryptoKey& publicKey) :

    TransactionMessage(
        equivalent,
        senderUUID,
        transactionUUID),
    mState(state),
    mPublicKey(publicKey)
{}

FinalAmountsConfigurationResponseMessage::FinalAmountsConfigurationResponseMessage(
    BytesShared buffer):

    TransactionMessage(buffer)
{
    size_t bytesBufferOffset = TransactionMessage::kOffsetToInheritedBytes();
    //----------------------------------------------------
    SerializedOperationState *state = new (buffer.get() + bytesBufferOffset) SerializedOperationState;
    mState = (OperationState) (*state);
    if (mState == Accepted) {
        bytesBufferOffset += sizeof(SerializedOperationState);
        size_t publicKeySize;
        memcpy(
            &publicKeySize,
            buffer.get() + bytesBufferOffset,
            sizeof(size_t));
        bytesBufferOffset += sizeof(size_t);

        mPublicKey.deserialize(
            publicKeySize,
            buffer.get() + bytesBufferOffset);
    } else {
        mPublicKey = CryptoKey();
    }
}

const FinalAmountsConfigurationResponseMessage::OperationState FinalAmountsConfigurationResponseMessage::state() const
{
    return mState;
}

const CryptoKey& FinalAmountsConfigurationResponseMessage::publicKey() const
{
    return mPublicKey;
}

/**
 *
 * @throws bad_alloc;
 */
pair<BytesShared, size_t> FinalAmountsConfigurationResponseMessage::serializeToBytes() const
    throw (bad_alloc)
{
    auto parentBytesAndCount = TransactionMessage::serializeToBytes();

    size_t bytesCount =
        parentBytesAndCount.second
        + sizeof(SerializedOperationState);

    BytesShared dataBytesShared = tryMalloc(bytesCount);
    size_t dataBytesOffset = 0;
    //----------------------------------------------------
    memcpy(
        dataBytesShared.get(),
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second);
    dataBytesOffset += parentBytesAndCount.second;
    //----------------------------------------------------
    SerializedOperationState state(mState);
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        &state,
        sizeof(SerializedOperationState));
    //----------------------------------------------------
    if (mState == Accepted) {
        auto publicKeySize = mPublicKey.keySize();
        memcpy(
            dataBytesShared.get() + dataBytesOffset,
            &publicKeySize,
            sizeof(size_t));
        dataBytesOffset += sizeof(size_t);

        memcpy(
            dataBytesShared.get() + dataBytesOffset,
            mPublicKey.key(),
            publicKeySize);
    }
    return make_pair(
        dataBytesShared,
        bytesCount);
}

const Message::MessageType FinalAmountsConfigurationResponseMessage::typeID() const
{
    return Message::Payments_FinalAmountsConfigurationResponse;
}