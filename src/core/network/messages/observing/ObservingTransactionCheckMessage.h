#ifndef GEO_NETWORK_CLIENT_OBSERVINGTRANSACTIONCHECKMESSAGE_H
#define GEO_NETWORK_CLIENT_OBSERVINGTRANSACTIONCHECKMESSAGE_H

#include "../Message.hpp"
#include "../../../transactions/transactions/base/TransactionUUID.h"

class ObservingTransactionCheckMessage : public Message {

public:
    typedef shared_ptr<ObservingTransactionCheckMessage> Shared;

public:
    ObservingTransactionCheckMessage(
        const TransactionUUID& transactionUUID);

    const MessageType typeID() const;

    pair<BytesShared, size_t> serializeToBytes() const;

private:
    TransactionUUID mTransactionUUID;
};


#endif //GEO_NETWORK_CLIENT_OBSERVINGTRANSACTIONCHECKMESSAGE_H
