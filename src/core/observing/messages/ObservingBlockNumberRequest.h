#ifndef GEO_NETWORK_CLIENT_OBSERVINGBLOCKNUMBERREQUEST_H
#define GEO_NETWORK_CLIENT_OBSERVINGBLOCKNUMBERREQUEST_H

#include "base/ObservingMessage.hpp"

class ObservingBlockNumberRequest : public ObservingMessage {

public:
    typedef shared_ptr<ObservingBlockNumberRequest> Shared;

    using ObservingMessage::ObservingMessage;

    const MessageType typeID() const override;
};


#endif //GEO_NETWORK_CLIENT_OBSERVINGBLOCKNUMBERREQUEST_H
