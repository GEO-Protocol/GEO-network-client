#include "ObservingBlockNumberRequest.h"

const ObservingMessage::MessageType ObservingBlockNumberRequest::typeID() const
{
    return ObservingMessage::Observing_BlockNumberRequest;
}