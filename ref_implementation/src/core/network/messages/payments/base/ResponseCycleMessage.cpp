/**
 * This file is part of GEO Protocol.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Protocol/GEO-network-client/blob/master/LICENSE.md
 *
 * No part of GEO Protocol, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#include "ResponseCycleMessage.h"

ResponseCycleMessage::ResponseCycleMessage(
    const NodeUUID& senderUUID,
    const TransactionUUID& transactionUUID,
    const OperationState state) :

    TransactionMessage(
        senderUUID,
        transactionUUID),
    mState(state)
{}

ResponseCycleMessage::ResponseCycleMessage(
    BytesShared buffer):

    TransactionMessage(buffer)
{
    size_t bytesBufferOffset = TransactionMessage::kOffsetToInheritedBytes();
    //----------------------------------------------------
    SerializedOperationState *state = new (buffer.get() + bytesBufferOffset) SerializedOperationState;
    mState = (OperationState) (*state);
}

const ResponseCycleMessage::OperationState ResponseCycleMessage::state() const
{
    return mState;
}

const size_t ResponseCycleMessage::kOffsetToInheritedBytes() const
    noexcept
{
    return TransactionMessage::kOffsetToInheritedBytes()
           + sizeof(SerializedOperationState);
}

/**
 *
 * @throws bad_alloc;
 */
pair<BytesShared, size_t> ResponseCycleMessage::serializeToBytes() const
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
    return make_pair(
        dataBytesShared,
        bytesCount);
}
