#ifndef GEO_NETWORK_CLIENT_OBSERVINGRESPONSEMESSAGE_H
#define GEO_NETWORK_CLIENT_OBSERVINGRESPONSEMESSAGE_H

#include "../../ObservingTransaction.h"

class ObservingResponseMessage {

public:
    ObservingResponseMessage(
        BytesShared buffer);

    const size_t kOffsetToInheritedBytes() const;

private:
    const ObservingTransaction::SerializedObservingResponseType kObserverCommunicatorMinimalError = 200;

protected:
    ObservingTransaction::SerializedObservingResponseType mObservingResponse;
};


#endif //GEO_NETWORK_CLIENT_OBSERVINGRESPONSEMESSAGE_H
