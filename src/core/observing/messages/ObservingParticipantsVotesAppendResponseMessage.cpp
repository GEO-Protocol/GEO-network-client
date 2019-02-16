#include "ObservingParticipantsVotesAppendResponseMessage.h"

ObservingParticipantsVotesAppendResponseMessage::ObservingParticipantsVotesAppendResponseMessage(
    BytesShared buffer) :
    ObservingResponseMessage(
        buffer)
{}

ObservingTransaction::ObservingResponseType ObservingParticipantsVotesAppendResponseMessage::observingResponse() const
{
    return (ObservingTransaction::ObservingResponseType)mObservingResponse;
}