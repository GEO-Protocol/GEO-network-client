#include "ObservingClaimAppendResponseMessage.h"

ObservingClaimAppendResponseMessage::ObservingClaimAppendResponseMessage(
    BytesShared buffer) :
    ObservingResponseMessage(
        buffer)
{}

ObservingTransaction::ObservingResponseType ObservingClaimAppendResponseMessage::observingResponse() const
{
    return (ObservingTransaction::ObservingResponseType)mObservingResponse;
}