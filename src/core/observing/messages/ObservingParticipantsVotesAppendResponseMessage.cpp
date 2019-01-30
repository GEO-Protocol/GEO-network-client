#include "ObservingParticipantsVotesAppendResponseMessage.h"

ObservingParticipantsVotesAppendResponseMessage::ObservingParticipantsVotesAppendResponseMessage(
    BytesShared buffer)
{
    auto *state = new (buffer.get()) ObservingTransaction::ObservingResponseType;
    mObservingResponse = *state;
}

const ObservingMessage::MessageType ObservingParticipantsVotesAppendResponseMessage::typeID() const
{
    return ObservingMessage::Observing_ParticipantsVotesAppendResponse;
}

ObservingTransaction::ObservingResponseType ObservingParticipantsVotesAppendResponseMessage::observingResponse() const
{
    return mObservingResponse;
}