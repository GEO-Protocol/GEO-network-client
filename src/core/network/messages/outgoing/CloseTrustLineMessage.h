#ifndef GEO_NETWORK_CLIENT_CLOSETRUSTLINEMESSAGE_H
#define GEO_NETWORK_CLIENT_CLOSETRUSTLINEMESSAGE_H

#include "../Message.h"

#include "../../../transactions/TransactionUUID.h"
#include "../../../common/NodeUUID.h"

class CloseTrustLineMessage: public Message {

public:
    CloseTrustLineMessage(
        NodeUUID &sender,
        TransactionUUID &transactionUUID,
        NodeUUID &contractorUUID
    );

    pair<ConstBytesShared, size_t> serialize();

    void deserialize(
        byte* buffer);

    const MessageTypeID typeID() const;

private:
    NodeUUID mContractorUUID;

};


#endif //GEO_NETWORK_CLIENT_CLOSETRUSTLINEMESSAGE_H
