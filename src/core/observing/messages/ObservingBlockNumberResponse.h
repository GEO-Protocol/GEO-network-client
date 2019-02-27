#ifndef GEO_NETWORK_CLIENT_OBSERVINGBLOCKNUMBERRESPONSE_H
#define GEO_NETWORK_CLIENT_OBSERVINGBLOCKNUMBERRESPONSE_H

#include "base/ObservingResponseMessage.h"

class ObservingBlockNumberResponse : public ObservingResponseMessage {

public:
    typedef shared_ptr<ObservingBlockNumberResponse> Shared;

public:
    ObservingBlockNumberResponse(
        BytesShared buffer);

    BlockNumber actualBlockNumber() const;

private:
    ObservingTransaction::SerializedObservingResponseType mObservingResponse;
    BlockNumber mActualBlockNumber;
};


#endif //GEO_NETWORK_CLIENT_OBSERVINGBLOCKNUMBERRESPONSE_H
