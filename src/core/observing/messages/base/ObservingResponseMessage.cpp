#include "ObservingResponseMessage.h"

ObservingResponseMessage::ObservingResponseMessage(
    BytesShared buffer)
{
    memcpy(
        &mObservingResponse,
        buffer.get(),
        sizeof(ObservingTransaction::SerializedObservingResponseType));
    if (mObservingResponse >= kObserverCommunicatorMinimalError) {
        throw ValueError("Wrong observing communicator response " + to_string((uint16_t)mObservingResponse));
    }
}

const size_t ObservingResponseMessage::kOffsetToInheritedBytes() const
{
    return sizeof(ObservingTransaction::SerializedObservingResponseType);
}