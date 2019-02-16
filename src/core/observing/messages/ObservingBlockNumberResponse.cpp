#include "ObservingBlockNumberResponse.h"

ObservingBlockNumberResponse::ObservingBlockNumberResponse(
    BytesShared buffer) :
    ObservingResponseMessage(
        buffer)
{
   memcpy(
        &mActualBlockNumber,
        buffer.get() + ObservingResponseMessage::kOffsetToInheritedBytes(),
        sizeof(BlockNumber));
}

BlockNumber ObservingBlockNumberResponse::actualBlockNumber() const
{
    return mActualBlockNumber;
}