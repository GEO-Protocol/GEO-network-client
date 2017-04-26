#ifndef GEO_NETWORK_CLIENT_VOTESSTATUSREQUESTMESSAGE_H
#define GEO_NETWORK_CLIENT_VOTESSTATUSREQUESTMESSAGE_H

#include "../base/transaction/TransactionMessage.h"


class VotesStatusRequestMessage :
        public TransactionMessage {

public:
    VotesStatusRequestMessage(
            const NodeUUID &senderUUID,
            const TransactionUUID &transactionUUID) noexcept :
            TransactionMessage(
                senderUUID,
                transactionUUID){};

public:
    const MessageType typeID() const{
        return Message::MessageType::Payments_VoutesStatusRequest;
    };
};


#endif //GEO_NETWORK_CLIENT_VOTESSTATUSREQUESTMESSAGE_H
