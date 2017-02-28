#ifndef GEO_NETWORK_CLIENT_RECEIVEROPERATIONSTATUSMESSAGE_H
#define GEO_NETWORK_CLIENT_RECEIVEROPERATIONSTATUSMESSAGE_H

#include "../../base/transaction/TransactionMessage.h"


class ReceiverApprovePaymentMessage:
    public TransactionMessage {

public:
    enum OperationState {
        Accepted = 1,
        Rejected = 2,
    };

    typedef byte SerializedOperationState;
    typedef shared_ptr<ReceiverApprovePaymentMessage> Shared;
    typedef shared_ptr<const ReceiverApprovePaymentMessage> ConstShared;

public:
    ReceiverApprovePaymentMessage(
        const NodeUUID &senderUUID,
        const TransactionUUID &transactionUUID,
        const OperationState state);

    ReceiverApprovePaymentMessage(
        BytesShared buffer);

    const OperationState state() const;

private:
    const MessageType typeID() const;

    pair<BytesShared, size_t> serializeToBytes();

    void deserializeFromBytes(
        BytesShared buffer);

private:
    mutable OperationState mState;
};
#endif //GEO_NETWORK_CLIENT_RECEIVEROPERATIONSTATUSMESSAGE_H
