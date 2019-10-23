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
        TrustLineIsAbsent = 4,
        TrustLineAlreadyPresent = 5,
        TrustLineInvalidState = 6,
        ReservationsPresentOnTrustLine = 7,
        Audit_Reject = 8,
        Audit_Invalid = 9,
        Audit_KeyNotFound = 10,
        Audit_IncorrectNumber = 11,
        OwnKeysAbsent = 12,
        ContractorKeysAbsent = 13,
    };

public:
    ConfirmationMessage(
        const SerializedEquivalent equivalent,
        const TransactionUUID &transactionUUID,
        const OperationState state = OK);

    ConfirmationMessage(
        const SerializedEquivalent equivalent,
        ContractorID idOnReceiverSide,
        const TransactionUUID &transactionUUID,
        const OperationState state = OK);

    ConfirmationMessage(
        BytesShared buffer);

    const MessageType typeID() const override;

    const OperationState state() const;

    pair<BytesShared, size_t> serializeToBytes() const override;

protected:
    const size_t kOffsetToInheritedBytes() const override;

private:
    typedef byte SerializedOperationState;

private:
    OperationState mState;
};

#endif // CONFIRMATIONMESSAGE_H
