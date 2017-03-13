#include "BasePaymentTransaction.h"

BasePaymentTransaction::BasePaymentTransaction(
    const TransactionType type,
    const NodeUUID &currentNodeUUID,
    TrustLinesManager *trustLines,
    Logger *log) :

    BaseTransaction(
        type,
        currentNodeUUID,
        log),
    mTrustLines(trustLines)
{
    cout << "\n\n BASE: " << mTrustLines << "\n\n" << endl;
}

BasePaymentTransaction::BasePaymentTransaction(
    const TransactionType type,
    const TransactionUUID &transactionUUID,
    const NodeUUID &currentNodeUUID,
    TrustLinesManager *trustLines,
    Logger *log) :

    BaseTransaction(
        type,
        transactionUUID,
        currentNodeUUID,
        log),
    mTrustLines(trustLines)
{
    cout << "\n\n BASE: " << mTrustLines << "\n\n" << endl;
}

BasePaymentTransaction::BasePaymentTransaction(
    const TransactionType type,
    BytesShared buffer,
    TrustLinesManager *trustLines,
    Logger *log) :

    BaseTransaction(
        type,
        log),
    mTrustLines(trustLines)
{
    cout << "\n\n BASE: " << mTrustLines << "\n\n" << endl;
}

const bool BasePaymentTransaction::reserveAmount(
    const NodeUUID& neighborNode,
    const TrustLineAmount& amount)
{
    try {
        // TODO: store reservation into internal storage to be able to serialize/deserialize it.
        mTrustLines->reserveAmount(
            neighborNode,
            UUID(),
            amount);
        return true;

    } catch (Exception &) {}

    return false;
}

const bool BasePaymentTransaction::reserveIncomingAmount(
    const NodeUUID& neighborNode,
    const TrustLineAmount& amount)
{
    try {
        // TODO: store reservation into internal storage to be able to serialize/deserialize it.
        mTrustLines->reserveIncomingAmount(
            neighborNode,
            UUID(),
            amount);
        return true;

    } catch (Exception &) {}

    return false;
}

const bool BasePaymentTransaction::validateContext(
    Message::MessageTypeID messageType) const
{
    if (mContext.empty()) {
        error() <<"Transaction context is empty. No messages was received. Canceling.";
        return false;
    }


    if (mContext.size() > 1 || mContext.at(0)->typeID() != messageType) {
        error() << "Unexpected message received. "
                   "It seems that remote node doesn't follows the protocol. "
                   "Canceling.";

        return false;
    }

    return true;
}
