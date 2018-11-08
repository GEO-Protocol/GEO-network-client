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
        TrustLineAlreadyPresent = 4,
        ReservationsPresentOnTrustLine = 5,
        Audit_Reject = 6,
        Audit_Invalid = 7,
        Audit_KeyNotFound = 8,
        Audit_IncorrectNumber = 9,
        OwnKeysAbsent = 10,
        ContractorKeysAbsent = 11,
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
