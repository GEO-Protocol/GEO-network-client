#ifndef GEO_NETWORK_CLIENT_OBSERVINGBLOCKNUMBERRESPONSE_H
#define GEO_NETWORK_CLIENT_OBSERVINGBLOCKNUMBERRESPONSE_H

#include "ObservingMessage.hpp"

class ObservingBlockNumberResponse : public ObservingMessage {

public:
    typedef shared_ptr<ObservingBlockNumberResponse> Shared;

public:
    ObservingBlockNumberResponse(
        BytesShared buffer);

    const MessageType typeID() const override;

    BlockNumber actualBlockNumber() const;

private:
    BlockNumber mActualBlockNumber;
};


#endif //GEO_NETWORK_CLIENT_OBSERVINGBLOCKNUMBERRESPONSE_H
