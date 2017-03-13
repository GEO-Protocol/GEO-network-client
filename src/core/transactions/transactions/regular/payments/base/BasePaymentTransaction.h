#ifndef BASEPAYMENTTRANSACTION_H
#define BASEPAYMENTTRANSACTION_H


#include "../../../base/BaseTransaction.h"

#include "../../../../../common/Types.h"
#include "../../../../../paths/lib/Path.h"

#include "../../../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../../../logger/Logger.h"


// TODO: Add restoring of the reservations after transaction deserialization.
class BasePaymentTransaction:
    public BaseTransaction {

public:
    BasePaymentTransaction(
        const TransactionType type,
        const NodeUUID &currentNodeUUID,
        TrustLinesManager *trustLines,
        Logger *log);

    BasePaymentTransaction(
        const TransactionType type,
        const TransactionUUID &transactionUUID,
        const NodeUUID &currentNodeUUID,
        TrustLinesManager *trustLines,
        Logger *log);

    BasePaymentTransaction(
        const TransactionType type,
        BytesShared buffer,
        TrustLinesManager *trustLines,
        Logger *log);

protected:
//    ConstSharedTrustLineAmount availableAmount(
//        const NodeUUID &neighborNode);

//    ConstSharedTrustLineAmount availableIncomingAmount(
//        const NodeUUID &neighborNode);

    const bool reserveAmount(
        const NodeUUID &neighborNode,
        const TrustLineAmount& amount);

    const bool reserveIncomingAmount(
        const NodeUUID &neighborNode,
        const TrustLineAmount& amount);

    const bool validateContext(
        Message::MessageTypeID messageType) const;

protected:
    // Specifies how long node must wait for the response from the remote node.
    // This timeout must take into account also that remote node may process other transaction,
    // and may be too busy to response.
    // (it is not only network transfer timeout).
    static const auto kMaxMessageTransferLagMSec = 1500; // msec

    static const auto kMaxNodesCount = 6;

protected:
    TrustLinesManager *mTrustLines;
};

#endif // BASEPAYMENTTRANSACTION_H
