#ifndef GEO_NETWORK_CLIENT_TRANSACTIONMESSAGE_H
#define GEO_NETWORK_CLIENT_TRANSACTIONMESSAGE_H

#include "../../SenderMessage.h"

#include "../../../../transactions/transactions/base/TransactionUUID.h"


using namespace std;


class TransactionMessage:
    public SenderMessage {

public:
    typedef shared_ptr<TransactionMessage> Shared;
    typedef shared_ptr<const TransactionMessage> ConstShared;

public:
    const TransactionUUID &transactionUUID() const;

public:
    TransactionMessage();

    TransactionMessage(
        const NodeUUID &senderUUID,
        const TransactionUUID &transactionUUID);

    TransactionMessage(
        BytesShared bufer);

    virtual const MessageType typeID() const = 0;

    virtual pair<BytesShared, size_t> serializeToBytes();

protected:
    virtual void deserializeFromBytes(
        BytesShared buffer);

    static const size_t kOffsetToInheritedBytes();

private:
    const bool isTransactionMessage() const;

protected:
    TransactionUUID mTransactionUUID;
};
#endif //GEO_NETWORK_CLIENT_TRANSACTIONMESSAGE_H
