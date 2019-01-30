#include "ObservingBlockNumberResponse.h"

ObservingBlockNumberResponse::ObservingBlockNumberResponse(
    BytesShared buffer)
{
    memcpy(
        &mActualBlockNumber,
        buffer.get(),
        sizeof(BlockNumber));
}

const ObservingMessage::MessageType ObservingBlockNumberResponse::typeID() const
{
    return ObservingMessage::Observing_BlockNumberResponse;
}

BlockNumber ObservingBlockNumberResponse::actualBlockNumber() const
{
    return mActualBlockNumber;
}