#ifndef GEO_NETWORK_CLIENT_NEIGHBORSREQUESTMESSAGE_H
#define GEO_NETWORK_CLIENT_NEIGHBORSREQUESTMESSAGE_H

#include "../base/transaction/TransactionMessage.h"


/*
 * This message is used for requesting remote node to send it's neighbors.
 */
class NeighborsRequestMessage:
    public TransactionMessage {

public:
    using TransactionMessage::TransactionMessage;

    const MessageType typeID() const
        noexcept;
};


#endif //GEO_NETWORK_CLIENT_NEIGHBORSREQUESTMESSAGE_H
