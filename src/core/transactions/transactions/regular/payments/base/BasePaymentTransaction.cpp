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
