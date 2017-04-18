#ifndef INTERMEDIATENODERESERVATIONRESPONSEMESSAGE_H
#define INTERMEDIATENODERESERVATIONRESPONSEMESSAGE_H


#include "base/ResponseMessage.h"


class IntermediateNodeReservationResponseMessage:
    public ResponseMessage {

public:
    typedef shared_ptr<IntermediateNodeReservationResponseMessage> Shared;
    typedef shared_ptr<const IntermediateNodeReservationResponseMessage> ConstShared;

public:
    using ResponseMessage::ResponseMessage;

private:
    const MessageType typeID() const;
};

#endif // INTERMEDIATENODERESERVATIONRESPONSEMESSAGE_H
