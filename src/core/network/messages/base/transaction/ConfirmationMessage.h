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
        ContractorBanned = 3,
        ReservationsPresentOnTrustLine = 4,
        Audit_Reject = 5,
        Audit_Invalid = 6,
        Audit_KeyNotFound = 7,
    };

public:
    ConfirmationMessage(
        const SerializedEquivalent equivalent,
        const NodeUUID &senderUUID,
        const TransactionUUID &transactionUUID,
        const OperationState state = OK);

    ConfirmationMessage(
        BytesShared buffer);

    const MessageType typeID() const;

    const OperationState state() const;

    pair<BytesShared, size_t> serializeToBytes() const
        throw (bad_alloc);

protected:
    const size_t kOffsetToInheritedBytes() const
        noexcept;

private:
    typedef byte SerializedOperationState;

private:
    OperationState mState;
};

#endif // CONFIRMATIONMESSAGE_H
