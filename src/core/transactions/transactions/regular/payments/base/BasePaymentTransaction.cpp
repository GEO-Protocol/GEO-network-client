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
{}

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
{}

BasePaymentTransaction::BasePaymentTransaction(
    const TransactionType type,
    BytesShared buffer,
    TrustLinesManager *trustLines,
    Logger *log) :

    BaseTransaction(
        type,
        log),
    mTrustLines(trustLines)
{}

ConstSharedTrustLineAmount BasePaymentTransaction::availableAmount(
    const NodeUUID& neighborNode)
{
    try {
        const auto tl = mTrustLines->trustLineReadOnly(neighborNode);
        return tl->availableAmount();

    } catch (NotFoundError &) {
        static auto zero = make_shared<const TrustLineAmount>(0);
        return zero;
    }
}

ConstSharedTrustLineAmount BasePaymentTransaction::availableIncomingAmount(
    const NodeUUID& neighborNode)
{
    try {
        const auto tl = mTrustLines->trustLineReadOnly(neighborNode);
        return tl->availableIncomingAmount();

    } catch (NotFoundError &) {
        static auto zero = make_shared<const TrustLineAmount>(0);
        return zero;
    }
}

const bool BasePaymentTransaction::reserveAmount(
    const NodeUUID& neighborNode,
    const TrustLineAmount& amount)
{
    try {
        const auto a = availableAmount(neighborNode);
        if (*a >= amount) {
            // TODO: store reservation into internal storage to be able to serialize/deserialize it.
            mTrustLines->reserveAmount(
                neighborNode,
                UUID(),
                amount);
            return true;
        }
    } catch (Exception &) {}

    return false;
}

const bool BasePaymentTransaction::reserveIncomingAmount(
    const NodeUUID& neighborNode,
    const TrustLineAmount& amount)
{
    try {
        const auto a = availableIncomingAmount(neighborNode);
        if (*a >= amount) {
            // TODO: store reservation into internal storage to be able to serialize/deserialize it.
            mTrustLines->reserveIncomingAmount(
                neighborNode,
                UUID(),
                amount);
            return true;
        }
    } catch (Exception &) {}

    return false;
}

TransactionResult::SharedConst BasePaymentTransaction::exit() const
{
    return make_shared<TransactionResult>(
        TransactionState::exit());
}
