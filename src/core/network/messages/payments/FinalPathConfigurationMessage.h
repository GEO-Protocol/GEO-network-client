#ifndef GEO_NETWORK_CLIENT_FINALPATHCONFIGURATIONMESSAGE_H
#define GEO_NETWORK_CLIENT_FINALPATHCONFIGURATIONMESSAGE_H

#include "base/RequestMessage.h"

class FinalPathConfigurationMessage : public RequestMessage {

public:
    typedef shared_ptr<FinalPathConfigurationMessage> Shared;
    typedef shared_ptr<const FinalPathConfigurationMessage> ConstShared;

public:
    using RequestMessage::RequestMessage;

protected:
    const MessageType typeID() const;
};


#endif //GEO_NETWORK_CLIENT_FINALPATHCONFIGURATIONMESSAGE_H
