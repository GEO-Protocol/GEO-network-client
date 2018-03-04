#ifndef CONFIRMATIONMESSAGE_H
#define CONFIRMATIONMESSAGE_H

#include "TransactionMessage.h"


class ConfirmationMessage:
    public TransactionMessage {

public:
    typedef shared_ptr<ConfirmationMessage> Shared;

public:
    enum OperationState {
        OK = 1,
        ErrorShouldBeRemovedFromQueue = 2,
        ContractorBanned = 3
    };

public:
    ConfirmationMessage(
        const SerializedEquivalent equivalent,
        const NodeUUID &senderUUID,
        const TransactionUUID &transactionUUID,
        const OperationState state = OK);

    ConfirmationMessage(
        BytesShared buffer);

    const MessageType typeID() const
        noexcept;

    const OperationState state() const;

private:
    typedef byte SerializedOperationState;

    pair<BytesShared, size_t> serializeToBytes() const
    throw (bad_alloc);

private:
    OperationState mState;
};

#endif // CONFIRMATIONMESSAGE_H
