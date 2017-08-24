#ifndef GEO_NETWORK_CLIENT_FINALAMOUNTSCONFIGURATIONMESSAGE_H
#define GEO_NETWORK_CLIENT_FINALAMOUNTSCONFIGURATIONMESSAGE_H

#include "base/RequestMessageWithReservations.h"

class FinalAmountsConfigurationMessage : public RequestMessageWithReservations {
public:
    typedef shared_ptr<FinalAmountsConfigurationMessage> Shared;
    typedef shared_ptr<const FinalAmountsConfigurationMessage> ConstShared;

public:
    using RequestMessageWithReservations::RequestMessageWithReservations;

    const MessageType typeID() const;
};


#endif //GEO_NETWORK_CLIENT_FINALAMOUNTSCONFIGURATIONMESSAGE_H
