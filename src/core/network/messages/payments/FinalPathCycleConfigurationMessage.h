#ifndef GEO_NETWORK_CLIENT_FINALPATHCYCLECONFIGURATIONMESSAGE_H
#define GEO_NETWORK_CLIENT_FINALPATHCYCLECONFIGURATIONMESSAGE_H

#include "base/RequestCycleMessage.h"

class FinalPathCycleConfigurationMessage :
    public RequestCycleMessage {

public:
    typedef shared_ptr<FinalPathCycleConfigurationMessage> Shared;
    typedef shared_ptr<const FinalPathCycleConfigurationMessage> ConstShared;

public:
    using RequestCycleMessage::RequestCycleMessage;

protected:
    const MessageType typeID() const;
};


#endif //GEO_NETWORK_CLIENT_FINALPATHCYCLECONFIGURATIONMESSAGE_H
