#ifndef GEO_NETWORK_CLIENT_RESERVATIONSINRELATIONTONODEMESSAGE_H
#define GEO_NETWORK_CLIENT_RESERVATIONSINRELATIONTONODEMESSAGE_H

#include "base/RequestMessageWithReservations.h"

class ReservationsInRelationToNodeMessage : public RequestMessageWithReservations {

public:
    typedef shared_ptr<ReservationsInRelationToNodeMessage> Shared;
    typedef shared_ptr<const ReservationsInRelationToNodeMessage> ConstShared;

public:
    using RequestMessageWithReservations::RequestMessageWithReservations;

protected:
    const MessageType typeID() const;
};


#endif //GEO_NETWORK_CLIENT_RESERVATIONSINRELATIONTONODEMESSAGE_H
