#include "ObservingClaimAppendResponseMessage.h"

ObservingClaimAppendResponseMessage::ObservingClaimAppendResponseMessage(
    BytesShared buffer)
{
    auto *state = new (buffer.get()) ObservingTransaction::SerializedObservingResponseType;
    mObservingResponse = (ObservingTransaction::ObservingResponseType)*state;
}

const ObservingMessage::MessageType ObservingClaimAppendResponseMessage::typeID() const
{
    return ObservingMessage::Observing_ClaimAppendResponse;
}

ObservingTransaction::ObservingResponseType ObservingClaimAppendResponseMessage::observingResponse() const
{
    return mObservingResponse;
}