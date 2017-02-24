#ifndef GEO_NETWORK_CLIENT_COORDINATORPAYMENTTRANSCATION_H
#define GEO_NETWORK_CLIENT_COORDINATORPAYMENTTRANSCATION_H

#include "../../../../common/Types.h"
#include "../../../../paths/lib/Path.h"

#include "../../base/BaseTransaction.h"
#include "../../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../../logger/Logger.h"

#include "../../../../interface/commands_interface/commands/payments/CreditUsageCommand.h"
#include "../../../../network/messages/outgoing/payments/ReceiverInitPaymentMessage.h"

#include <map>


/**
 * Contains path and max capabilities amount
 * that is common for all the nodes in this path.
 */
class PaymentPath {
public:
    typedef unique_ptr<PaymentPath> Unique;
    typedef uint16_t Identifier; // TODO: change me to sha256

public:
    PaymentPath(
        Path &path,
        Identifier identifier);

    const bool wasUsedForAmountReservation() const;

protected:
    Path mPath;

    // Identifies payment path within payment operation.
    //
    // It's common for the payment operation to
    // contains several (thousands?) payment paths.
    //
    // Paths must be distinquished by the sha256 hash.
    // sha256 must be gnerated from the nodes uuids and random salt.
    // Initiator must not accept responses from the intermediate node,
    // in case if previous attempt was invalid (protocol error).
    Identifier mIdentifier;

    // "true" if this path was already used for amount blocking.
    // "false" - otherwise.
    bool mIsAmountBlocksSent;

    TrustLineAmount mMaxCommonCapabilities;
};


class PaymentPathsHandler {
public:
    void add(
        Path &path);

    const PaymentPath& nextNotReservedPaymentPath() const;
    const bool empty() const;

protected:
    vector<PaymentPath::Unique> mPaths;
};


class CoordinatorPaymentTransaction:
    public BaseTransaction {

public:
    typedef shared_ptr<CoordinatorPaymentTransaction> Shared;
    typedef shared_ptr<const CoordinatorPaymentTransaction> ConstShared;

public:
    CoordinatorPaymentTransaction(
        const NodeUUID &currentNodeUUID,
        CreditUsageCommand::Shared command,
        TrustLinesManager *trustLines,
        Logger *log);

    CoordinatorPaymentTransaction(
        BytesShared buffer,
        TrustLinesManager *trustLines,
        Logger *log);

    TransactionResult::SharedConst run();

    pair<BytesShared, size_t> serializeToBytes();
    const string logHeader() const;

protected:
    // Stages handlers
    TransactionResult::SharedConst initTransaction();
    TransactionResult::SharedConst processReceiverResponse();
    TransactionResult::SharedConst tryBlockAmounts();

protected:
    // Results handlers
    TransactionResult::SharedConst resultOK();
    TransactionResult::SharedConst resultNoPaths();
    TransactionResult::SharedConst resultProtocolError();

    void deserializeFromBytes(
        BytesShared buffer);

protected:
//    void tryBlockAmountsOnIntermediateNodes();

protected:
    CreditUsageCommand::Shared mCommand;
    PaymentPathsHandler mPaymentPaths;

    TrustLinesManager *mTrustLines;
    Logger *mLog;
};
#endif //GEO_NETWORK_CLIENT_COORDINATORPAYMENTTRANSCATION_H
