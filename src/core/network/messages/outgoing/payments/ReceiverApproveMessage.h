#ifndef GEO_NETWORK_CLIENT_RECEIVEROPERATIONSTATUSMESSAGE_H
#define GEO_NETWORK_CLIENT_RECEIVEROPERATIONSTATUSMESSAGE_H

#include "../../base/transaction/TransactionMessage.h"

#include "../../../../common/Types.h"
#include "../../../../common/multiprecision/MultiprecisionUtils.h"

#include "../../../../common/exceptions/ValueError.h"


class ReceiverApproveMessage:
    public TransactionMessage {

public:
    enum OperationState {
        Accepted = 1,
        Rejected = 2,
    };

    typedef byte SerializedOperationState;
    typedef shared_ptr<ReceiverApproveMessage> Shared;
    typedef shared_ptr<const ReceiverApproveMessage> ConstShared;

public:
    ReceiverApproveMessage(
        const NodeUUID &senderUUID,
        const TransactionUUID &transactionUUID,
        const OperationState state);

    ReceiverApproveMessage(
        BytesShared buffer);

    pair<BytesShared, size_t> serializeToBytes();
    const MessageType typeID() const;
    const OperationState state() const;

    const TransactionUUID &transactionUUID() const { throw ValueError("");}
    const TrustLineUUID &trustLineUUID() const { throw ValueError("");}

protected:
    void deserializeFromBytes(
        BytesShared buffer);

private:
    mutable OperationState mState;
};


#endif //GEO_NETWORK_CLIENT_RECEIVEROPERATIONSTATUSMESSAGE_H
