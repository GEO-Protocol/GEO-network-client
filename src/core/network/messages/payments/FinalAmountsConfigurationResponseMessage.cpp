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
    const lamport::PublicKey::Shared publicKey) :

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
    auto *state = new (buffer.get() + bytesBufferOffset) SerializedOperationState;
    mState = (OperationState) (*state);
    if (mState == Accepted) {
        bytesBufferOffset += sizeof(SerializedOperationState);
        auto publicKey = make_shared<lamport::PublicKey>(
            buffer.get() + bytesBufferOffset);
        mPublicKey = publicKey;
    }
}

const FinalAmountsConfigurationResponseMessage::OperationState FinalAmountsConfigurationResponseMessage::state() const
{
    return mState;
}

const lamport::PublicKey::Shared FinalAmountsConfigurationResponseMessage::publicKey() const
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

    if (mState == Accepted) {
        bytesCount += mPublicKey->keySize();
    }

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
        dataBytesOffset += sizeof(SerializedOperationState);
        memcpy(
            dataBytesShared.get() + dataBytesOffset,
            mPublicKey->data(),
            mPublicKey->keySize());
    }
    return make_pair(
        dataBytesShared,
        bytesCount);
}

const Message::MessageType FinalAmountsConfigurationResponseMessage::typeID() const
{
    return Message::Payments_FinalAmountsConfigurationResponse;
}