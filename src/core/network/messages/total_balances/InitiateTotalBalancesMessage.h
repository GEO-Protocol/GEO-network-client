#ifndef GEO_NETWORK_CLIENT_INITIATETOTALBALANCESMESSAGE_H
#define GEO_NETWORK_CLIENT_INITIATETOTALBALANCESMESSAGE_H

#include "../SenderMessage.h"
#include "../../../common/multiprecision/MultiprecisionUtils.h"

class InitiateTotalBalancesMessage : public SenderMessage {

public:
    typedef shared_ptr<InitiateTotalBalancesMessage> Shared;

public:

    InitiateTotalBalancesMessage(
            const NodeUUID& senderUUID);

    InitiateTotalBalancesMessage(
            BytesShared buffer);

    const MessageType typeID() const;

    const bool isTotalBalancesResponseMessage() const;

};


#endif //GEO_NETWORK_CLIENT_INITIATETOTALBALANCESMESSAGE_H
