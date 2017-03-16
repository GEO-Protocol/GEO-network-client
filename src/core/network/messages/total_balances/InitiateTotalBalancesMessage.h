#ifndef GEO_NETWORK_CLIENT_INITIATETOTALBALANCESMESSAGE_H
#define GEO_NETWORK_CLIENT_INITIATETOTALBALANCESMESSAGE_H

#include "../../../common/multiprecision/MultiprecisionUtils.h"
#include "../base/transaction/TransactionMessage.h"

class InitiateTotalBalancesMessage : public TransactionMessage {

public:
    typedef shared_ptr<InitiateTotalBalancesMessage> Shared;

public:

    InitiateTotalBalancesMessage(
        const NodeUUID& senderUUID,
        const TransactionUUID &transactionUUID);

    InitiateTotalBalancesMessage(
        BytesShared buffer);

    const MessageType typeID() const;

    const bool isTotalBalancesResponseMessage() const;

};


#endif //GEO_NETWORK_CLIENT_INITIATETOTALBALANCESMESSAGE_H
