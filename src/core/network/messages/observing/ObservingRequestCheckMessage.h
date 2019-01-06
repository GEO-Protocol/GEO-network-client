#ifndef GEO_NETWORK_CLIENT_OBSERVINGREQUESTCHECKMESSAGE_H
#define GEO_NETWORK_CLIENT_OBSERVINGREQUESTCHECKMESSAGE_H

#include "../Message.hpp"
#include "../../../transactions/transactions/base/TransactionUUID.h"

class ObservingRequestCheckMessage : public Message {

public:
    typedef shared_ptr<ObservingRequestCheckMessage> Shared;

public:
    ObservingRequestCheckMessage(
        const TransactionUUID& transactionUUID);

    const MessageType typeID() const;

    pair<BytesShared, size_t> serializeToBytes() const;

private:
    TransactionUUID mTransactionUUID;
};


#endif //GEO_NETWORK_CLIENT_OBSERVINGREQUESTCHECKMESSAGE_H
