#include "ObservingClaimAppendResponseMessage.h"

ObservingClaimAppendResponseMessage::ObservingClaimAppendResponseMessage(
    BytesShared buffer)
{
    auto *state = new (buffer.get()) ObservingTransaction::ObservingResponseType;
    mObservingResponse = *state;
}

const ObservingMessage::MessageType ObservingClaimAppendResponseMessage::typeID() const
{
    return ObservingMessage::Observing_ClaimAppendResponse;
}

ObservingTransaction::ObservingResponseType ObservingClaimAppendResponseMessage::observingResponse() const
{
    return mObservingResponse;
}